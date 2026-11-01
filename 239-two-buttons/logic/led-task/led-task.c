#include "led-task/led-task.h"
#include "systime/systime.h"
#include "platform/platform.h"

#include <stdio.h>

// Состояние задачи: внутреннее связывание, только для этого файла
static led_state_t led_state;
static bool led_is_on = false;
static uint64_t half_period_us = 500000;
static uint64_t last_toggle_us = 0;

static const char *led_state_names[] = {
    "off", 
    "on", 
    "blink",
};

const char* led_task_get_state_name(led_state_t state)
{
    if (state < 0 || state > 3) return "unknown";
    return led_state_names[state];
}

static void led_set(bool state)
{
    led_is_on = state;
    platform_led_set(state);
}

void led_task_init(void)
{
    led_task_set_state(LED_STATE_ON);
}

void led_task_handle(void)
{
    switch (led_state) {
    case LED_STATE_OFF:
    case LED_STATE_ON:
        break;
    case LED_STATE_BLINK:
        if (systime_period_elapsed(&last_toggle_us, half_period_us)) {
            led_is_on = !led_is_on;
            platform_led_set(led_is_on);
        }
        break;
    }
}

void led_task_set_state(led_state_t state)
{
    led_state = state;

    // Действие при входе: выполняется один раз, в момент перехода
    switch (led_state) {
    case LED_STATE_OFF:
        led_set(false);
        break;
    case LED_STATE_ON:
        led_set(true);
        break;
    case LED_STATE_BLINK:
        // Первое переключение — на ближайшей итерации
        last_toggle_us = 0;
        break;
    }
}

led_state_t led_task_get_state(void)
{
    return led_state;
}

bool led_task_set_period_ms(uint32_t period_ms)
{
    // С нулевым полупериодом светодиод переключался бы
    // на каждой итерации: такой период не принимается
    if (period_ms == 0) {
        return false;
    }

    half_period_us = period_ms * 1000 / 2;
    return true;
}

uint32_t led_task_get_period_ms(void)
{
    return 2 * half_period_us / 1000;
}

void led_task_next_state(void)
{
    led_task_set_state(!led_state);
}

void led_period_250_500_1000(void)
{
    static uint32_t p_num = 2;
    uint32_t period = led_task_get_period_ms();

    if(period != 250 && period != 500 && period != 1000)
    {
        p_num = 0;
        led_task_set_period_ms(250);
    }
    else
    {
        if(p_num == 0)
        {
            p_num = 1;
            led_task_set_period_ms(500);
        }
        else if(p_num == 1)
        {
            p_num = 2;
            led_task_set_period_ms(1000);
        }
        else
        {
            p_num = 0;
            led_task_set_period_ms(250);
        }
    }

}