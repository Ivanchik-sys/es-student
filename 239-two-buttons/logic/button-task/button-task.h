#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BUTTON_COUNT 2

typedef void (*button_task_callback_t)(void);

void button_task_init(uint32_t id, button_task_callback_t on_press_short, button_task_callback_t on_press_long);

bool button_task_is_pressed(uint32_t id);
unsigned int button_task_get_press_count(uint32_t id);

void button_task_handle();
