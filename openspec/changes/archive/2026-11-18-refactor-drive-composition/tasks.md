## 1. Конфигурация

- [x] 1.1 Перенести пины энкодеров и моторов из `chassis.cpp`/`encoders.cpp` в `app_config.h` как `constexpr`; добавить `#include <Arduino.h>`.
- [x] 1.2 Перенести `kVMaxMmS` и `kWheelbaseMm` в `app_config.h`; удалить `drive_config.h`.
- [x] 1.3 Обновить `#include "drive_config.h"` → `#include "../app_config.h"` в `motor.cpp` и `chassis.cpp`.
- [x] 1.4 Обновить комментарий с путём к конфигурации в `py_scripts/pc_control/controller.py`.
- [x] 1.5 Собрать `pio run` (стенд: нет).

## 2. Энкодер и группа порта

- [x] 2.1 `Encoder(uint8_t pinA, uint8_t pinB)`: сохранить пины и маски, добавить `initPins()` и `updateFromPort(portBits)`, убрать `direction` из `measureSpeed()`.
- [x] 2.2 `Encoders(Encoder&, Encoder&)`: `init()` настраивает пины обоих и `PCIE1`, `handleInterrupt()` читает `PINC` один раз; `ISR(PCINT1_vect)` и `extern Encoders encoders` сохранить.
- [x] 2.3 Собрать `pio run` (стенд: нет).

## 3. Иерархия моторов

- [x] 3.1 Ввести базовый `Motor`: защищённый конструктор с аргументом знака, поле `const int8_t sign_`, перевод знака в `tick()` и `applyPwm()`.
- [x] 3.2 Добавить `LeftMotor` и `RightMotor`, передающие `-1`/`+1` в конструктор базы.
- [x] 3.3 Добавить `Motor::init()`: `pinMode` трёх пинов и `coast()`.
- [x] 3.4 Собрать `pio run`, сверить RAM/Flash с базовыми 22.0%/28.6% (стенд: нет).

## 4. Композиция

- [x] 4.1 Перевести `Chassis` на `Motor &left_, &right_`; убрать `Encoders`, `kMotorCount` и индексы `kLeftMotor/kRightMotor`.
- [x] 4.2 `Chassis::init()` вызывает `init()` обоих моторов; `fillTelemetry()` пишет два колеса явно.
- [x] 4.3 Переписать `main.cpp`: `Encoder` → `Encoders` → `LeftMotor/RightMotor` → `Chassis`; в `setup()` вызвать `encoders.init()` и `controller.init()`.
- [x] 4.4 Собрать `pio run` (стенд: нет).

## 5. Проверка и завершение

- [x] 5.1 Стендовый прогон: вперёд `mL,mR,oL,oR > 0`; назад — все `< 0` (стенд: да).
- [x] 5.2 Стендовый прогон: вращение на месте — колёса в противоположных направлениях, ручной проворот колеса вызывает противодействие (стенд: да).
- [x] 5.3 Сравнить телеметрию до/после на `set_speed 120 0` и `150 0`: знаки и величины полей, отсутствие положительной обратной связи (`oL=oR=255`) (стенд: да).
- [x] 5.4 Запустить `openspec validate --all --strict`.
- [x] 5.5 Обновить `openspec/backlog/backlog.md`: снять пункты о знаке, моторах в `main`, `drive_config`; отметить, что `Encoders` сохранён как группа порта.
