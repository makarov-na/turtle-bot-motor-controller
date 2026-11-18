// --- Мотор: интерфейс -------------------------------------------------------
// Motor — база одного колеса: скрывает энкодер, драйвер L298N, редуктор и
// состояние контура скорости. Знак колеса хранится в поле и задаётся
// наследником: LeftMotor или RightMotor. Состояние контура закрыто; менять
// его может только Chassis.
//
// Реализация — в motor.cpp.

#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

#include "encoders.h"

// Снимок состояния одного мотора для телеметрии. PID-внутренности остаются
// закрытыми, наружу отдаётся только то, что требует протокол ack.
struct MotorTelemetry {
  float measuredSpeed;
  float pwm;
  float integral;
};

class Motor {
 public:
  Motor(const Motor &) = delete;
  Motor &operator=(const Motor &) = delete;

  MotorTelemetry telemetry() const;

  // Настраивает три пина моста и переводит мотор в свободный выбег.
  void init();

 protected:
  // Знак колеса: +1 — положительный ШИМ ведёт робота вперёд; -1 — мотор
  // установлен зеркально. Передаётся наследником и дальше не меняется.
  Motor(uint8_t pwm, uint8_t in1, uint8_t in2, int8_t sign, Encoder &enc);

 private:
  friend class Chassis;

  void setTarget(float target);
  void tick(float dtSec);
  void rebaseEncoder();
  void resetPid();
  void resetIntegral();
  void brake();
  void coast();
  float pidUpdate(float setpoint, float feedback, float dt);
  void applyPwm(float pwmCmd);

  const uint8_t pinPwm_;
  const uint8_t pinIn1_;
  const uint8_t pinIn2_;
  const int8_t sign_;
  Encoder &encoder_;

  float targetLinear_ = 0.0f;
  float measuredLinear_ = 0.0f;
  float integral_ = 0.0f;
  float prevError_ = 0.0f;
  float lastOut_ = 0.0f;
};

// Левый мотор установлен зеркально.
class LeftMotor : public Motor {
 public:
  LeftMotor(uint8_t pwm, uint8_t in1, uint8_t in2, Encoder &enc)
      : Motor(pwm, in1, in2, -1, enc) {}
};

class RightMotor : public Motor {
 public:
  RightMotor(uint8_t pwm, uint8_t in1, uint8_t in2, Encoder &enc)
      : Motor(pwm, in1, in2, +1, enc) {}
};

#endif
