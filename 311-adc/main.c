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
#include "telemetry-task/telemetry-task.h"

typedef enum {
    PROFILING_LOOP,
    PROFILING_COMMAND,
    PROFILING_LED,
    PROFILING_BUTTON,
    PROFILING_PI,
    PROFILING_MAX_STOPWATCHES // Автоматически равен 5
} profiling_id_t;

int main()
{
    platform_init();
    led_task_init();
    button_task_init(led_task_next_state);
    telemetry_task_init(api_print_telemetry);

    #if PROFILING_ENABLED
        profiling_stopwatch_init(PROFILING_LOOP, "loop");
        profiling_stopwatch_init(PROFILING_COMMAND, "command");
        profiling_stopwatch_init(PROFILING_LED, "led");
        profiling_stopwatch_init(PROFILING_BUTTON, "button");
        profiling_stopwatch_init(PROFILING_PI, "pi");
    #endif

    LOG_INF("%s %s\n", FIRMWARE_NAME, FIRMWARE_VERSION);

    while (1)
    {
        profiling_start(PROFILING_LOOP);
        
        profiling_start(PROFILING_LED);
        led_task_handle();
        profiling_stop(PROFILING_LED);

        profiling_start(PROFILING_BUTTON);
        button_task_handle();
        profiling_stop(PROFILING_BUTTON);

        profiling_start(PROFILING_PI);
        pi_task_handle();
        profiling_stop(PROFILING_PI);

        telemetry_task_handle();
        
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