# Backlog

## Открытые задачи

- Добавить команды `stop` и `powerStop` в протокол для ПК.
- Долго выходим на целевое значение скорости — см. `wheel-speed-convergence.md`.
- Нестабильно работает скрипт подключения к ПК.

## Выполнено

- Работа с энкодерами вынесена в `Encoder` / `Encoders`.
- Привод вынесен в `Chassis`.
- Приём команд и watchdog вынесены в `CommandReceiver`.
- Телеметрия вынесена в `Telemetry` / `TelemetryFrame`.
- Автоконфигурация HC-06 вынесена в `BtConfigurator`.
- Композиция собрана в `MotorController`; скетч `.ino` заменён на `main.cpp` и `.cpp/.h`.
