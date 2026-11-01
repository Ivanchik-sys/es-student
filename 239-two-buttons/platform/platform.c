#include "platform/platform.h"
#include "pico/stdlib.h" 
#include "hardware/gpio.h"
#include <stdint.h>


#define PLATFORM_LED_PIN PICO_DEFAULT_LED_PIN 
#define PLATFORM_BUTTON_PIN_15 15
#define PLATFORM_BUTTON_PIN_14 14                   

void platform_init(void)
{
    stdio_init_all();

    gpio_init(PLATFORM_LED_PIN);
    gpio_set_dir(PLATFORM_LED_PIN, GPIO_OUT);

    gpio_init(PLATFORM_BUTTON_PIN_15);
    gpio_set_dir(PLATFORM_BUTTON_PIN_15, GPIO_IN);
    gpio_pull_up(PLATFORM_BUTTON_PIN_15);

    gpio_init(PLATFORM_BUTTON_PIN_14);
    gpio_set_dir(PLATFORM_BUTTON_PIN_14, GPIO_IN);
    gpio_pull_up(PLATFORM_BUTTON_PIN_14);
}

void platform_led_set(bool on)
{
    gpio_put(PLATFORM_LED_PIN, on);
}

bool platform_button_read(uint32_t id)
{
    switch(id)
    {
        case 0:
            return gpio_get(PLATFORM_BUTTON_PIN_15) == 0;
        case 1:
            return gpio_get(PLATFORM_BUTTON_PIN_14) == 0;
    }

    return false;
    
}
