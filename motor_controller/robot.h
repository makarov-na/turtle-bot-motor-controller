// --- Композиция модулей: интерфейс -----------------------------------------
// Robot — корень композиции ядровой логики: связывает привод, приёмник команд
// и телеметрию, повторяя прежний порядок работы loop(). CommandReceiver только
// разбирает протокол и сообщает о срабатывании watchdog; применение команд к
// Chassis — в robot.cpp.
//
// Экземпляр Robot создаётся в основном скетче последним и получает ссылки на
// уже созданные Chassis, CommandReceiver и Telemetry.

#ifndef ROBOT_H
#define ROBOT_H

#include <Arduino.h>

#include "command_receiver.h"
#include "chassis.h"
#include "src/telemetry/telemetry.h"

class Robot {
 public:
  Robot(Chassis &chassis, CommandReceiver &commands, Telemetry &telemetry);

  void init();
  void tick();

 private:
  Chassis &chassis_;
  CommandReceiver &commands_;
  Telemetry &telemetry_;
};

#endif
