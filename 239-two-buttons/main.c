#include <stdio.h>

#include "led-task/led-task.h"
#include "pi-task/pi-task.h"
#include "api/api.h"
#include "button-task/button-task.h"

#include "profiling/profiling.h"
#include "logging/log.h"
#include "stdio-text-protocol/stdio-text-protocol.h"
#include "firmware.h"
#include "platform/platform.h"

typedef enum {
    PROFILING_LOOP,
    PROFILING_COMMAND,
    PROFILING_LED,
    PROFILING_BUTTON_1,
    PROFILING_BUTTON_2,
    PROFILING_PI,
    PROFILING_MAX_STOPWATCHES
} profiling_id_t;

int main()
{
    platform_init();
    led_task_init();
    button_task_init(0, led_task_next_state, led_task_blink);
    button_task_init(1, led_period_250_500_1000, NULL);

    #if PROFILING_ENABLED
        profiling_stopwatch_init(PROFILING_LOOP, "loop");
        profiling_stopwatch_init(PROFILING_COMMAND, "command");
        profiling_stopwatch_init(PROFILING_LED, "led");
        profiling_stopwatch_init(PROFILING_BUTTON_1, "button 1");
        profiling_stopwatch_init(PROFILING_BUTTON_2, "button 2");
        profiling_stopwatch_init(PROFILING_PI, "pi");
    #endif

    LOG_INF("%s %s\n", FIRMWARE_NAME, FIRMWARE_VERSION);

    while (1)
    {
        profiling_start(PROFILING_LOOP);
        
        profiling_start(PROFILING_LED);
        led_task_handle();
        profiling_stop(PROFILING_LED);

        profiling_start(PROFILING_BUTTON_1);
        button_task_handle(0);
        profiling_stop(PROFILING_BUTTON_1);

        profiling_start(PROFILING_BUTTON_2);
        button_task_handle(1);
        profiling_stop(PROFILING_BUTTON_2);

        profiling_start(PROFILING_PI);
        pi_task_handle();
        profiling_stop(PROFILING_PI);
        
        profiling_start(PROFILING_COMMAND);
        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL)
        {
            api_handle(command);
        }
        profiling_stop(PROFILING_COMMAND);

        profiling_stop(PROFILING_LOOP);
    }
}