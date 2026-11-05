// --- Общая конфигурация приложения ------------------------------------------
// Настройки, общие для разных модулей. Файл лежит рядом с main.cpp.
// Модули из подкаталогов подключают его как "../app_config.h".

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>

// Скорость Hardware Serial: USB-монитор, HC-06 и ПК-пульт.
constexpr uint32_t kSerialBaud = 115200;

#endif
