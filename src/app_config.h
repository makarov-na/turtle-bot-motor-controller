// --- Общая конфигурация приложения ------------------------------------------
// Настройки связи, параметры привода и распиновка платы. Файл лежит рядом с
// main.cpp; модули из подкаталогов подключают его как "../app_config.h".

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <Arduino.h>

// --- Связь -------------------------------------------------------------------

// Скорость Hardware Serial: USB-монитор, HC-06 и ПК-пульт.
constexpr uint32_t kSerialBaud = 115200;

// Автоконфигурация HC-06 при старте: перевод модуля на kSerialBaud.
// false — модуль должен быть уже настроен на kSerialBaud вручную.
constexpr bool kEnableBtAutoConfig = false;

// --- Привод ------------------------------------------------------------------

// Максимальная линейная скорость середины оси колёс, мм/с.
constexpr float kVMaxMmS = 300.0f;

// База колёс, мм.
constexpr float kWheelbaseMm = 185.0f;

// --- Распиновка --------------------------------------------------------------

// Энкодеры. Каналы A/B каждого колеса обязаны быть на PORTC (A0..A5):
// ISR обслуживает PCINT1_vect. Порядок A/B задаёт направление счёта.
constexpr uint8_t kPinEncRA = A0;
constexpr uint8_t kPinEncRB = A1;
constexpr uint8_t kPinEncLA = A2;
constexpr uint8_t kPinEncLB = A3;

// L298N: правый мотор.
constexpr uint8_t kPinIn1R = 2;
constexpr uint8_t kPinIn2R = 3;
constexpr uint8_t kPinPwmR = 5;

// L298N: левый мотор.
constexpr uint8_t kPinIn3L = 4;
constexpr uint8_t kPinIn4L = 7;
constexpr uint8_t kPinPwmL = 6;

#endif
