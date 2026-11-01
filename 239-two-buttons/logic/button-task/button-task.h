#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BUTTON_COUNT 2

typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS_Q,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE_Q
} button_state_t;

typedef void (*button_task_callback_t)(void);

typedef struct {
    button_state_t state;
    uint64_t timer;
    uint32_t presses;
    button_task_callback_t press_callback_short;
    button_task_callback_t press_callback_long;
    uint64_t timer_long;
    bool press_long;
} button_params_t;


void button_task_init(uint32_t id, button_task_callback_t on_press_short, button_task_callback_t on_press_long);

bool button_task_is_pressed(uint32_t id);
unsigned int button_task_get_press_count(uint32_t id);

void button_task_handle(uint32_t id);
