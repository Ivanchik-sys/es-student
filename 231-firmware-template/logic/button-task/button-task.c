#include "button-task/button-task.h"
#include <stdio.h>
#include "systime/systime.h"
#include "platform/platform.h"


typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS_Q,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE_Q
} button_state_t;

static button_state_t button_state;
static uint64_t button_timer = 0;
static uint32_t button_presses = 0;
static uint64_t debounce = 20000;

static const char *button_state_names[] = {
    "released", 
    "released",
    "pressed",
    "pressed",
};

static button_task_callback_t press_callback = NULL;

void button_task_init(button_task_callback_t on_press)
{
    button_state = BUTTON_STATE_RELEASED;
    button_timer = 0;
    button_presses = 0;
    
    press_callback = on_press;
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
    bool is_button_down = platform_button_read();
    
    // читаем часы
    uint64_t now_us = systime_us();

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
                     // ВАЖНО: Фиксируем нажатие и вызываем колбэк прибора!
                    if (press_callback != NULL) 
                        press_callback();
                    
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
