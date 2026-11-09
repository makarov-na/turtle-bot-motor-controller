// --- Приём и разбор команд, watchdog: реализация ---------------------------
// CommandReceiver разбирает только синтаксис строки: set_speed v w, stop,
// power_stop. Watchdog остановлен, пока не пришла первая валидная команда;
// каждая валидная команда сбрасывает таймер. CommandReceiver только сообщает
// о срабатывании, остановку выполняет MotorController.

#include "command_receiver.h"

#include <stdlib.h>
#include <string.h>

namespace {

const uint32_t kWatchdogTimeoutMs = 1000;

}  // namespace

CommandReceiver::CommandReceiver(Stream &serial) : serial_(&serial) {}

bool CommandReceiver::poll(uint32_t now, Command &out) {
  while (serial_->available() > 0) {
    char c = (char)serial_->read();
    if (c == '\n' || c == '\r') {
      bool valid =
          cmdLen_ > 0 && !cmdOverflow_ && parseCommand_(cmdBuf_, now, out);
      // Сбрасываем буфер и признак переполнения в любом случае: длинная
      // битая строка не должна навсегда залипать в парсере.
      cmdLen_ = 0;
      cmdOverflow_ = false;
      if (valid) {
        return true;
      }
    } else if (cmdLen_ < kBufSize - 1) {
      cmdBuf_[cmdLen_++] = c;
    } else {
      cmdOverflow_ = true;
    }
  }
  return false;
}

bool CommandReceiver::parseCommand_(char *buf, uint32_t now, Command &out) {
  // Парсим вручную: scanf-вариант из avr-libc по умолчанию не умеет %f
  // (нужна принудительная линковка libscanf_flt), а strtod доступен всегда.
  char *saveptr = nullptr;
  char *tok = strtok_r(buf, " ", &saveptr);
  if (!tok) {
    return false;
  }

  if (strcmp(tok, "stop") == 0 || strcmp(tok, "power_stop") == 0) {
    // Остановки — ровно один токен, без аргументов и хвостов.
    if (strtok_r(nullptr, " ", &saveptr) != nullptr) {
      return false;
    }
    out.action =
        (strcmp(tok, "power_stop") == 0) ? Action::PowerStop : Action::Stop;
    out.linear = 0.0f;
    out.angular = 0.0f;
  } else if (strcmp(tok, "set_speed") == 0) {
    char *vTok = strtok_r(nullptr, " ", &saveptr);
    char *wTok = strtok_r(nullptr, " ", &saveptr);
    char *extra = strtok_r(nullptr, " ", &saveptr);
    if (!vTok || !wTok || extra != nullptr) {
      return false;
    }
    char *endV = nullptr;
    char *endW = nullptr;
    float v = (float)strtod(vTok, &endV);
    float w = (float)strtod(wTok, &endW);
    if (endV == vTok || endW == wTok || *endV != '\0' || *endW != '\0') {
      return false;
    }
    out.action = Action::SetSpeed;
    out.linear = v;
    out.angular = w;
  } else {
    return false;
  }

  lastCommandMs_ = now;
  watchdogTripped_ = false;

  cmdCounter_++;
  lastLinear_ = out.linear;
  lastAngular_ = out.angular;
  return true;
}

bool CommandReceiver::watchdogExpired(uint32_t now) {
  // Беззнаковая разность защищает от переполнения millis().
  if (!watchdogTripped_ &&
      (uint32_t)(now - lastCommandMs_) > kWatchdogTimeoutMs) {
    watchdogTripped_ = true;
    return true;
  }
  return false;
}

uint32_t CommandReceiver::counter() const {
  return cmdCounter_;
}

float CommandReceiver::lastLinear() const {
  return lastLinear_;
}

float CommandReceiver::lastAngular() const {
  return lastAngular_;
}
