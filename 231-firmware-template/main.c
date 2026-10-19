#include "led-task.h"
#include "log.h"
#include "device.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pi-task.h"
#include "profiling.h"
#include "api.h"
#include "button-task.h"

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
    stdio_text_protocol_init();   

    led_task_init();

    button_task_init();

    profiling_stopwatch_init(PROFILING_LOOP, "loop");
    profiling_stopwatch_init(PROFILING_COMMAND, "command");
    profiling_stopwatch_init(PROFILING_LED, "led");
    profiling_stopwatch_init(PROFILING_BUTTON, "button");
    profiling_stopwatch_init(PROFILING_PI, "pi");

    LOG_INF("%s %s\n", DEVICE_NAME, FIRMWARE_VERSION);

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