#include "pi-task/pi-task.h"
#include <stdint.h>
#include <stdio.h>
#include "systime/systime.h"

// 125e6 тактов/s * 0.5 * 1e-3 s / 585 тактов ~= 106 членов
#define PI_TERMS_PER_CALL 100

// Состояние расчёта: должно пережить возврат из обработчика
static pi_state_t pi_state = PI_NOT_STARTED;
static uint32_t pi_k;
static uint32_t pi_n;
static double pi_sum;
static double pi_sign;
static double pi_result;
static uint64_t time_start;
static uint64_t time_finish;


bool pi_task_start(uint32_t n)
{
    if(pi_state != PI_RUNNING) {
        pi_n = n;
        pi_k = 0;
        pi_sum = 0.0;
        pi_sign = 1.0;
        time_start = systime_us();
        pi_state = PI_RUNNING;
        return true;
    }
    return false;
}

bool pi_task_get_result(double *result, unsigned int *time_ms)
{
    if (pi_state == PI_READY) {
        if (result) 
        {
            *result = pi_result;
        }
        if (time_ms) 
        {
            *time_ms = (unsigned int)((time_finish - time_start) / 1000);
        }
        return true;
    }
    return false;
}

void pi_task_handle(void)
{
    
    switch(pi_state)
    {
        case PI_NOT_STARTED:
            break;
        case PI_RUNNING:
            int end = pi_k + PI_TERMS_PER_CALL;
            if (end > pi_n) {
            end = pi_n;
            }
            for (; pi_k < end; pi_k++) {
                pi_sum += pi_sign / (2.0 * pi_k + 1.0);
                pi_sign = -pi_sign;
            }

            if (pi_k == pi_n) {
                pi_result = 4.0 * pi_sum;
                pi_state = PI_READY;
                time_finish = systime_us();
            }
            break;
        case PI_READY:
            break;
    }
    
}

pi_state_t pi_task_get_state(void)
{
    return pi_state;
}

