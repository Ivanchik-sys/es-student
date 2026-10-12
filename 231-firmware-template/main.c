#include "led.h"
#include "log.h"
#include "device.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "profiling.h"
#include "api.h"


int main()
{
    stdio_text_protocol_init();   

    profiling_init();

    while (1)
    {

        profiling_iteration();
        
        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL)
        {
            api_handle(command);
        }
    }
}