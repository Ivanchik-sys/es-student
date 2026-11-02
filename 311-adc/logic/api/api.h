#pragma once

#include "stdio-text-protocol/stdio-text-protocol.h"
#include "telemetry-task/telemetry-task.h"

// обработчик команды: получает имя и аргументы
typedef void (*api_callback_t)(const command_t *command);

void api_handle(const command_t *command);

// печатает запись телеметрии строкой tm: <время, мкс> <напряжение, В> <температура, °C>
void api_print_telemetry(const telemetry_record_t *record);