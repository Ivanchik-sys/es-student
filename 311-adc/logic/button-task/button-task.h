#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef void (*button_task_callback_t)(void);

void button_task_init(button_task_callback_t on_press);

bool button_task_is_pressed(void);
unsigned int button_task_get_press_count(void);

void button_task_handle(void);
