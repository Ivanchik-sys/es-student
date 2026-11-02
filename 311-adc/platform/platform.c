#include "platform/platform.h"
#include "pico/stdlib.h" 
#include "hardware/gpio.h"
#include "hardware/adc.h"

// вход АЦП: вывод GP26, канал 0
#define ADC_PIN 26
#define ADC_CHANNEL 0

// встроенный датчик температуры — канал 4
#define ADC_TEMP_CHANNEL 4

// опорное напряжение АЦП на плате Pico и число уровней 12-разрядного кода
#define ADC_VREF_V 3.3f
#define ADC_LEVELS 4096

#define PLATFORM_LED_PIN PICO_DEFAULT_LED_PIN 
#define PLATFORM_BUTTON_PIN 15                   

void platform_init(void)
{
    stdio_init_all();

    gpio_init(PLATFORM_LED_PIN);
    gpio_set_dir(PLATFORM_LED_PIN, GPIO_OUT);

    gpio_init(PLATFORM_BUTTON_PIN);
    gpio_set_dir(PLATFORM_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(PLATFORM_BUTTON_PIN);

    adc_init();
    adc_gpio_init(ADC_PIN);

    adc_set_temp_sensor_enabled(true);
}

void platform_led_set(bool on)
{
    gpio_put(PLATFORM_LED_PIN, on);
}

bool platform_button_read(void)
{
    return gpio_get(PLATFORM_BUTTON_PIN) == 0;
}

float platform_voltage_read(void)
{
    adc_select_input(ADC_CHANNEL);
    return adc_read() * ADC_VREF_V / ADC_LEVELS;
}

float platform_temp_read(void)
{
    adc_select_input(ADC_TEMP_CHANNEL);
    return 27 - (adc_read() * ADC_VREF_V / ADC_LEVELS - 0.706) / 0.001721;
}