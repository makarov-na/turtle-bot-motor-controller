
* Добавить команды stop и powerStop в протокол для порта
* Рефакторинг - вынести в отдельный класс работу с энкодерами 
* Долго выходим на целевое значение скорости
* Не стабильно работает скрипт подключения к PC

Предлагаемый порядок работ
1. Безопасность Encoder — атомарный takeTicks(), приватный update(), удаление неиспользуемого ticks(), запрет копирования. Маленький коммит, поведение не меняется.
2. Chassis — вынести driveState, таймеры, насыщение, set_speed/stop/power_stop, tick() и снимок телеметрии. Самый большой выигрыш по инкапсуляции.
3. CommandReceiver + CommandSink — убрать глобалы парсера и watchdog, развязать парсинг и привод.
4. Telemetry + TelemetryFrame — класс с приватным таймером, формат кадра не меняется.
5. BtConfigurator + BtResult — убрать btUseCrlf, magic-коды и макрос.
6. .h/.cpp + Robot — отдельный change, уже без изменения поведения.


#include "src/bluetooth/bt_config.h"
#include "src/protocol/command_receiver.h"
#include "motor_controller.h"
#include "src/chassis/chassis.h"
#include "src/chassis/encoders.h"
#include "src/telemetry/telemetry.h"


Encoders encoders;
Chassis chassis(encoders);
CommandReceiver commands(Serial);
Telemetry telemetry(Serial);
MotorController controller(chassis, commands, telemetry);
BtConfigurator bt(Serial, LED_BUILTIN);

void setup() {
controller.init();
bt.init();
}

void loop() {
controller.tick();
}
