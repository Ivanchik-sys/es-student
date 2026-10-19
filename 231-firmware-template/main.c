#include "led-task.h"
#include "log.h"
#include "device.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "profiling.h"
#include "api.h"
#include "button-task.h"


int main()
{
    stdio_text_protocol_init();   

    profiling_init();

    led_task_init();

    button_task_init();

    while (1)
    {

        profiling_iteration();
        led_task_handle();
        button_task_handle();
        
        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL)
        {
            api_handle(command);
        }
    }
}