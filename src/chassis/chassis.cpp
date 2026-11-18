// --- Привод целиком: реализация ---------------------------------------------

#include "chassis.h"

#include <math.h>

#include "../app_config.h"

namespace {

const float kOmegaMaxRadS = 2.0f * kVMaxMmS / kWheelbaseMm;

const uint32_t kBrakeDurationMs = 300;
const uint32_t kPidPeriodMs = 10;

}  // namespace

Chassis::Chassis(Motor &left, Motor &right) : left_(left), right_(right) {}

void Chassis::init() {
  left_.init();
  right_.init();
  state_ = DriveState::Stopped;
}

void Chassis::tick(uint32_t now) {
  if (state_ == DriveState::Braking &&
      now - brakeStartMs_ >= kBrakeDurationMs) {
    state_ = DriveState::Stopped;
    left_.coast();
    right_.coast();
  }

  if (state_ == DriveState::Run) {
    controlStep_();
  }
}

void Chassis::set_speed(float linearSpeed, float angularSpeed) {
  float vRight = linearSpeed + angularSpeed * kWheelbaseMm / 2.0f;
  float vLeft = linearSpeed - angularSpeed * kWheelbaseMm / 2.0f;

  float maxAbs = fmaxf(fabsf(vLeft), fabsf(vRight));
  if (maxAbs > kVMaxMmS) {
    float k = kVMaxMmS / maxAbs;
    vLeft *= k;
    vRight *= k;
  }

  // Вход в режим управления после паузы: перебазировать опорную точку
  // контура, чтобы первая итерация не увидела накопленные за паузу тики и
  // время. Команда поверх уже идущего Run (heartbeat) НЕ перебазирует:
  // иначе таймер контура сбрасывался бы каждые 200 мс и controlStep_() не
  // успевал бы отработать.
  if (state_ != DriveState::Run) {
    lastControlMs_ = millis();
    left_.rebaseEncoder();
    right_.rebaseEncoder();
  }

  state_ = DriveState::Run;
  left_.setTarget(vLeft);
  right_.setTarget(vRight);
}

void Chassis::stop() {
  state_ = DriveState::Stopped;
  left_.setTarget(0.0f);
  right_.setTarget(0.0f);
  left_.coast();
  right_.coast();
  left_.resetPid();
  right_.resetPid();
}

void Chassis::power_stop() {
  left_.setTarget(0.0f);
  right_.setTarget(0.0f);
  left_.resetPid();
  right_.resetPid();
  state_ = DriveState::Braking;
  brakeStartMs_ = millis();
  left_.brake();
  right_.brake();
}

void Chassis::fillTelemetry(TelemetryFrame &frame) const {
  MotorTelemetry left = left_.telemetry();
  MotorTelemetry right = right_.telemetry();
  frame.measured[0] = left.measuredSpeed;
  frame.measured[1] = right.measuredSpeed;
  frame.pwm[0] = left.pwm;
  frame.pwm[1] = right.pwm;
  frame.integral[0] = left.integral;
  frame.integral[1] = right.integral;
}

void Chassis::controlStep_() {
  uint32_t now = millis();
  uint32_t gap = now - lastControlMs_;
  if (gap < kPidPeriodMs) {
    return;
  }
  // Длинная итерация во время активного управления (например, блокирующая
  // печать кадра телеметрии ~78 мс) — это НЕ пауза: колесо всё это время
  // вращалось, дельта тиков настоящая. Меряем по реальному dt и ничего не
  // подменяем нулём. Настоящая пауза (торможение/останов/watchdog) видна по
  // смене состояния и обрабатывается перебазированием в set_speed().
  float dt = (float)gap / 1000.0f;
  // Дедлайн следующей итерации — от текущего момента, без захода в будущее:
  // иначе uint32_t-разность на следующей итерации переполняется и даёт
  // ложный огромный gap.
  lastControlMs_ = now;

  left_.tick(dt);
  right_.tick(dt);
}
