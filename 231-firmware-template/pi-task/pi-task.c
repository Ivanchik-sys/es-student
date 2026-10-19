#include "pi-task.h"
#include "pico/stdlib.h"
#include <stdint.h>
#include <stdio.h>

typedef enum {
    PI_NOT_STARTED,
    PI_RUNNING,
    PI_READY
} pi_state_t;

// 125e6 тактов/s * 0.5 * 1e-3 s / 585 тактов ~= 106 членов
#define PI_TERMS_PER_CALL 100

// Состояние расчёта: должно пережить возврат из обработчика
static pi_state_t pi_state;
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
        time_start = time_us_64();
        pi_state = PI_RUNNING;
        return true;
    }
    return false;
}

bool pi_task_get_result(double *result, unsigned int *time)
{
    switch(pi_state)
    {
        case PI_NOT_STARTED:
            printf("pi: not started\n");
            return false;
        case PI_RUNNING:
            printf("pi: running\n");
            return false;
        case PI_READY:
            *result = pi_result;
            *time = (unsigned int) ((time_finish - time_start) / 1000);
            printf("pi: %f in %u ms\n", *result, *time);
            return true;
    }
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
                time_finish = time_us_64();
            }
            break;
        case PI_READY:
            break;
    }
    
}

