#pragma once

#include "stdio-text-protocol.h"

// обработчик команды: получает имя и аргументы
typedef void (*api_callback_t)(const command_t *command);

// команда прибора: имя, справка для help и обработчик
typedef struct
{
    const char *name;
    const char *help;
    api_callback_t callback;
} api_command_t;

typedef enum {
    PROFILING_LOOP,
    PROFILING_COMMAND,
    PROFILING_LED,
    PROFILING_BUTTON,
    PROFILING_PI,
    PROFILING_MAX_STOPWATCHES // Автоматически станет равен 5
} profiling_id_t;

void api_handle(const command_t *command);