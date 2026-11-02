#include "telemetry-task/telemetry-task.h"

#include "systime/systime.h"
#include "platform/platform.h"
#include <stdio.h>

static telemetry_task_callback_t telemetry_record_callback = NULL;

static telemetry_state_t telemetry_state;
static telemetry_record_t telemetry_record;
static uint64_t telemetry_last_toggle;
static uint64_t telemetry_period;

void telemetry_task_init(telemetry_task_callback_t on_record)
{
    telemetry_state = TELEMETRY_STATE_IDLE;
    telemetry_record_callback = on_record;
}

bool telemetry_task_start(uint32_t period_ms)
{
    if(period_ms < 10)
    {
        return false;
    }

    telemetry_state = TELEMETRY_STATE_RUN;
    telemetry_period = (uint64_t) (1000 * period_ms);
    telemetry_last_toggle = 0;
    return true;
}

void telemetry_task_stop(void)
{
    telemetry_state = TELEMETRY_STATE_IDLE;
}

telemetry_state_t telemetry_task_get_state(void)
{
    return telemetry_state;
}

uint32_t telemetry_task_get_period_ms(void)
{
    return (uint32_t) (1e-3 * telemetry_period);
}

void telemetry_task_handle()
{

    switch(telemetry_state)
    {
        case TELEMETRY_STATE_IDLE:
            break;

        case TELEMETRY_STATE_RUN:
            if(systime_period_elapsed(&telemetry_last_toggle, telemetry_period))
            {
                telemetry_record.time_us = systime_us();
                telemetry_record.voltage_v = platform_voltage_read();
                telemetry_record.temp_c = platform_temp_read();
                telemetry_record_callback(&telemetry_record);
            }
    }
}