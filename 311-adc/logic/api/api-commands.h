#pragma once

#include "api/api.h"

// Тип записи таблицы команд прибора
typedef struct {
    const char *name;
    const char *help;
    api_callback_t callback;
} api_command_t;

// Объявление внешней таблицы команд и её размера
extern const api_command_t api_commands[];
extern const uint32_t api_command_count;