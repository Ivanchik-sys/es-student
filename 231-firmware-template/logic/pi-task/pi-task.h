#pragma once
#include <stdint.h>


bool pi_task_start(uint32_t n);
bool pi_task_get_result(double *result, unsigned int *time);
void pi_task_handle(void);

