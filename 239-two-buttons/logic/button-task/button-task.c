#include "button-task/button-task.h"
#include <stdint.h>
#include <stdio.h>
#include "systime/systime.h"
#include "platform/platform.h"

static uint64_t debounce = 20000;

static button_params_t buttons[BUTTON_COUNT];

void button_task_init(uint32_t id, button_task_callback_t on_press)
{
    buttons[id].state = BUTTON_STATE_RELEASED;
    buttons[id].timer = 0;
    buttons[id].presses = 0;
    buttons[id].press_callback = on_press;
}

bool button_task_is_pressed(uint32_t id)
{
    return (buttons[id].state == BUTTON_STATE_PRESSED || buttons[id].state == BUTTON_STATE_RELEASE_Q);
}

unsigned int button_task_get_press_count(uint32_t id)
{
    return buttons[id].presses;
}

void button_task_handle(uint32_t id)
{
    // читаем уровень: нажатая кнопка замыкает вывод на землю и даёт ноль
    bool is_button_down = platform_button_read(id);
    
    // читаем часы
    uint64_t now_us = systime_us();

    switch (buttons[id].state)
    {
        case BUTTON_STATE_RELEASED:
        
            if (is_button_down) 
            {
                buttons[id].timer = now_us;
                buttons[id].state = BUTTON_STATE_PRESS_Q;
            }
            break;

        case BUTTON_STATE_PRESS_Q:
            if (is_button_down) 
            {
                // Если время дребезга прошло и кнопка всё ещё нажата
                if (now_us - buttons[id].timer >= debounce) 
                {
                    buttons[id].state = BUTTON_STATE_PRESSED;
                
                    // СТРОГО ПРИ ПЕРЕХОДЕ: увеличиваем счётчик и переключаем светодиод
                    buttons[id].presses++;
                     // ВАЖНО: Фиксируем нажатие и вызываем колбэк прибора!
                    if (buttons[id].press_callback != NULL) 
                        buttons[id].press_callback();
                    
                }
            } 
            else 
            {
                // Если кнопку отпустили раньше времени — это был шум
                buttons[id].state = BUTTON_STATE_RELEASED;
            }
            break;

        case BUTTON_STATE_PRESSED:
            // Если кнопку отпустили, запускаем таймер дребезга для отпускания
            if (!is_button_down) 
            {
                buttons[id].timer = now_us;
                buttons[id].state = BUTTON_STATE_RELEASE_Q;
            }
            break;

        case BUTTON_STATE_RELEASE_Q:
            if (!is_button_down) 
            {
                // Если время дребезга прошло и кнопку действительно отпустили
                if (now_us - buttons[id].timer >= debounce) 
                {
                    buttons[id].state = BUTTON_STATE_RELEASED;
                }
            } 
            else 
            {
                // Если кнопку снова зажали во время отпускания
                buttons[id].state = BUTTON_STATE_PRESSED;
            }
            break;
    }
}
