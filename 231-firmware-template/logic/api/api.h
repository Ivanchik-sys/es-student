#pragma once

#include "stdio-text-protocol/stdio-text-protocol.h"

// обработчик команды: получает имя и аргументы
typedef void (*api_callback_t)(const command_t *command);

void api_handle(const command_t *command);