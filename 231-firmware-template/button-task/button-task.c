#include "button-task.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "led-task.h"

static uint BUTTON_PIN = 15;
static button_state_t button_state;
static uint64_t button_timer = 0;
static uint button_presses = 0;
static uint64_t debounce = 20000;

static const char *button_state_names[] = {
    "released", 
    "released",
    "pressed",
    "pressed",
};

void button_task_init(void)
{
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    button_state = BUTTON_STATE_RELEASED;
}


bool button_task_is_pressed(void)
{
    return (button_state == BUTTON_STATE_PRESSED || button_state == BUTTON_STATE_RELEASE_Q);
}


unsigned int button_task_get_press_count(void)
{
    return button_presses;
}



void button_task_handle(void)
{
    // читаем уровень: нажатая кнопка замыкает вывод на землю и даёт ноль
    bool is_button_down = !gpio_get(BUTTON_PIN);
    
    // читаем часы
    uint64_t now_us = time_us_64();

    switch (button_state)
    {
        case BUTTON_STATE_RELEASED:
        
            if (is_button_down) 
            {
                button_timer = now_us;
                button_state = BUTTON_STATE_PRESS_Q;
            }
            break;

        case BUTTON_STATE_PRESS_Q:
            if (is_button_down) 
            {
                // Если время дребезга прошло и кнопка всё ещё нажата
                if (now_us - button_timer >= debounce) 
                {
                    button_state = BUTTON_STATE_PRESSED;
                
                    // СТРОГО ПРИ ПЕРЕХОДЕ: увеличиваем счётчик и переключаем светодиод
                    button_presses++;
                    led_task_next_state(); 
                }
            } 
            else 
            {
                // Если кнопку отпустили раньше времени — это был шум
                button_state = BUTTON_STATE_RELEASED;
            }
            break;

        case BUTTON_STATE_PRESSED:
            // Если кнопку отпустили, запускаем таймер дребезга для отпускания
            if (!is_button_down) 
            {
                button_timer = now_us;
                button_state = BUTTON_STATE_RELEASE_Q;
            }
            break;

        case BUTTON_STATE_RELEASE_Q:
            if (!is_button_down) 
            {
                // Если время дребезга прошло и кнопку действительно отпустили
                if (now_us - button_timer >= debounce) 
                {
                    button_state = BUTTON_STATE_RELEASED;
                }
            } 
            else 
            {
                // Если кнопку снова зажали во время отпускания
                button_state = BUTTON_STATE_PRESSED;
            }
            break;
    }
}
