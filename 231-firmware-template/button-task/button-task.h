#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS_Q,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE_Q
} button_state_t;

void button_task_init(void);

bool button_task_is_pressed(void);
unsigned int button_task_get_press_count(void);

void button_task_handle(void);
