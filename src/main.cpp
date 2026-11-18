#include <Arduino.h>
// Блок управления приводом
// Arduino Nano + L298N + моторы с энкодерами
//
// Команды в теле робота:
//   set_speed(linear_speed, angular_speed)
//     linear_speed  — линейная скорость середины оси колёс, мм/с
//     angular_speed — угловая скорость, рад/с; + = против часовой (вид сверху)
//   stop()       — отключить драйвер, свободный выбег
//   power_stop() — силовое торможение (L298N fast motor stop)
//
// Подключение и направления моторов сверять с
// https://github.com/makarov-na/turtle-bot/blob/refactoring/docs/4.%20Блок%20управления%20приводом/spec.md
//
// HC-06 Bluetooth: D0 (RX), D1 (TX), 115200 baud, Hardware Serial
//   HC-06 TX → D0 (напрямую, 3.3V читается как HIGH на 5V-логике)
//   HC-06 RX ← D1 (через делитель 5V → 3.3V, RX HC-06 не 5V-tolerant)
//   ВНИМАНИЕ: при перепрошивке через USB отключать HC-06 (загрузчик
//   использует D0/D1). SoftwareSerial не используется — его PCINT-алиасы
//   конфликтуют с ISR(PCINT1_vect) энкодеров на ATmega328P.
//
// Протокол управления по каналу (текст, \n — конец команды):
//   set_speed <linear_speed> <angular_speed>
//     linear_speed  — мм/с, любой знак
//     angular_speed — рад/с, любой знак
// Команды принимаются всегда. Watchdog: если команда не получена более
// 1000 мс, бот выполняет stop() до прихода новой команды.

#include "app_config.h"
#include "bluetooth/bt_config.h"
#include "protocol/command_receiver.h"
#include "motor_controller.h"
#include "chassis/chassis.h"
#include "chassis/encoders.h"
#include "chassis/motor.h"
#include "telemetry/telemetry.h"

// Модули — отдельные C++-единицы трансляции:
//   src/app_config.h — настройки связи, параметры привода и распиновка платы;
//   src/chassis/encoders.h/.cpp — Encoder/Encoders и ISR квадратурных энкодеров;
//   src/chassis/chassis.h/.cpp — Chassis: привод целиком, режимы и телеметрия;
//   src/chassis/motor.h/.cpp — Motor/LeftMotor/RightMotor: драйвер, энкодер,
//     знак колеса и контур скорости;
//   motor_controller.h/.cpp — MotorController: композиция ядровых модулей;
//   src/bluetooth/bt_config.h/.cpp — автоконфигурация HC-06;
//   src/protocol/command_receiver.h/.cpp — приём, разбор команд и watchdog;
//   src/telemetry/telemetry.h/.cpp — TelemetryFrame и периодический кадр ack.
//
// Здесь — корень композиции и точки входа Arduino. Порядок объявления =
// порядок конструирования: энкодеры → группа порта → моторы → шасси →
// остальные модули.
Encoder leftEncoder(kPinEncLA, kPinEncLB);
Encoder rightEncoder(kPinEncRA, kPinEncRB);
Encoders encoders(leftEncoder, rightEncoder);

LeftMotor leftMotor(kPinEnbL, kPinIn3L, kPinIn4L, leftEncoder);
RightMotor rightMotor(kPinEnaR, kPinIn1R, kPinIn2R, rightEncoder);

Chassis chassis(leftMotor, rightMotor);

CommandReceiver commands(Serial);
Telemetry telemetry(Serial);
MotorController controller(chassis, commands, telemetry);
BtConfigurator bt(Serial, LED_BUILTIN);

void setup() {
  // UART для телеметрии и команд. bt.init() тоже поднимает Serial, но сейчас
  // автоконфигурация HC-06 отключена, поэтому инициализация нужна явно.
  Serial.begin(kSerialBaud);

  // PORTC/PCINT1: пины энкодеров как входы, маска и разрешение прерывания.
  encoders.init();

  // Инициализация железа — явная, после конструирования всех модулей.
  controller.init();

  // Автоконфигурация HC-06: поднимает UART и переводит модуль на рабочую
  // скорость. Состояние после этого живёт в самом модуле и Hardware Serial.
  if (kEnableBtAutoConfig) {
    bt.init();
  }
}

void loop() {
  controller.tick();
}
