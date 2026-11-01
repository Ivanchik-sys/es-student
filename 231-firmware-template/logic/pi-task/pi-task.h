#pragma once
#include <stdint.h>

typedef enum {
    PI_NOT_STARTED,
    PI_RUNNING,
    PI_READY
} pi_state_t;

bool pi_task_start(uint32_t n);
bool pi_task_get_result(double *result, unsigned int *time);
void pi_task_handle(void);

bool pi_task_get_result(double *result, unsigned int *time_ms);
pi_state_t pi_task_get_state(void);
