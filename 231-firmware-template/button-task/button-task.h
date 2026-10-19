#pragma once

#include <stdbool.h>
#include <stdint.h>


void button_task_init(void);

bool button_task_is_pressed(void);
unsigned int button_task_get_press_count(void);

void button_task_handle(void);
