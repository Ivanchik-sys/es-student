#include "edge-task/edge-task.h"
#include "systime/systime.h"
#include <stddef.h>

#define EMPTY_V     0.05f
#define RISE_V      2.0f
#define FULL_V      3.2f
#define FALL_V      1.3f
#define TIMEOUT_US  2000000ULL

typedef enum {
    EDGE_STATE_IDLE,
    EDGE_STATE_DISCHARGE, // Разряд конденсатора перед замером < 0.05 В
    EDGE_STATE_RISE,      // Замер времени нарастания >= 2.0 В
    EDGE_STATE_CHARGE,    // Дозаряд конденсатора перед спадом >= 3.2 В
    EDGE_STATE_FALL       // Замер времени спада до <= 1.3 В
} edge_state_t;

static edge_state_t state = EDGE_STATE_IDLE;
static platform_line_mode_t current_mode;
static edge_task_callback_t done_callback = NULL;
static uint64_t phase_start_us = 0;
static uint32_t rise_time_us = 0;

static void enter(edge_state_t next_state, bool level)
{
    platform_line_set(current_mode, level);
    phase_start_us = systime_us();
    state = next_state;
}

static void finish(bool ok, uint32_t fall_us)
{
    edge_task_callback_t cb = done_callback;
    platform_line_set(current_mode, false);
    done_callback = NULL;
    state = EDGE_STATE_IDLE;

    if (cb != NULL)
    {
        cb(current_mode, ok, rise_time_us, fall_us);
    }
}

bool edge_task_start(platform_line_mode_t mode, edge_task_callback_t on_done)
{
    if (state != EDGE_STATE_IDLE || on_done == NULL)
    {
        return false;
    }

    current_mode = mode;
    done_callback = on_done;
    rise_time_us = 0;

    enter(EDGE_STATE_DISCHARGE, false);
    return true;
}

void edge_task_handle(void)
{
    if (state == EDGE_STATE_IDLE)
    {
        return;
    }

    uint64_t elapsed_us = systime_us() - phase_start_us;
    if (elapsed_us > TIMEOUT_US)
    {
        finish(false, 0);
        return;
    }

    float voltage_v = platform_voltage_read();

    switch (state)
    {
        case EDGE_STATE_DISCHARGE:
            if (voltage_v < EMPTY_V)
            {
                enter(EDGE_STATE_RISE, true);
            }
            break;

        case EDGE_STATE_RISE:
            if (voltage_v >= RISE_V)
            {
                rise_time_us = (uint32_t)elapsed_us;
                enter(EDGE_STATE_CHARGE, true);
            }
            break;

        case EDGE_STATE_CHARGE:
            if (voltage_v >= FULL_V)
            {
                enter(EDGE_STATE_FALL, false);
            }
            break;

        case EDGE_STATE_FALL:
            if (voltage_v <= FALL_V)
            {
                finish(true, (uint32_t)elapsed_us);
            }
            break;

        case EDGE_STATE_IDLE:
            break;
    }
}