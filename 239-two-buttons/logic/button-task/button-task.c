#include "button-task/button-task.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "systime/systime.h"
#include "platform/platform.h"

typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS_Q,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE_Q
} button_state_t;

typedef struct {
    button_state_t state;
    uint64_t timer;
    uint32_t presses;
    button_task_callback_t callback_short;
    button_task_callback_t callback_long;
    uint64_t timer_long;
    bool press_long;
} button_params_t;

static uint64_t debounce = 20000;
static uint64_t t_press_long = 1000000;

static button_params_t buttons[BUTTON_COUNT];

void button_task_init(uint32_t id, button_task_callback_t on_press_short, button_task_callback_t on_press_long)
{
    if (id >= BUTTON_COUNT || id < 0) 
        return;
    
    buttons[id].state = BUTTON_STATE_RELEASED;
    buttons[id].timer = 0;
    buttons[id].presses = 0;
    buttons[id].callback_short = on_press_short;
    buttons[id].callback_long = on_press_long;
    buttons[id].timer_long = 0;
    buttons[id].press_long = false;
}

bool button_task_is_pressed(uint32_t id)
{
    return (buttons[id].state == BUTTON_STATE_PRESSED || buttons[id].state == BUTTON_STATE_RELEASE_Q);
}

unsigned int button_task_get_press_count(uint32_t id)
{
    return buttons[id].presses;
}

void button_task_handle()
{
    for(uint32_t id = 0; id < BUTTON_COUNT; id++)
    {
        
        bool is_button_down = platform_button_read(id);
        
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
                    
                    if (now_us - buttons[id].timer >= debounce) 
                    {
                        buttons[id].state = BUTTON_STATE_PRESSED;
                        buttons[id].timer_long = buttons[id].timer;
                        
                        buttons[id].presses++;

                        if (buttons[id].callback_short != NULL) 
                            buttons[id].callback_short();

                    }
                } 
                else 
                {
                    buttons[id].state = BUTTON_STATE_RELEASED;
                }
                break;

            case BUTTON_STATE_PRESSED:
                if (!is_button_down) 
                {
                    buttons[id].timer = now_us;
                    buttons[id].state = BUTTON_STATE_RELEASE_Q;
                }
                else if(!buttons[id].press_long && now_us - buttons[id].timer_long >= t_press_long)
                {
                    buttons[id].press_long = true;
                    if (buttons[id].callback_long != NULL) 
                        buttons[id].callback_long();
                }
                break;

            case BUTTON_STATE_RELEASE_Q:
                if (!is_button_down) 
                {
                    if (now_us - buttons[id].timer >= debounce) 
                    {
                        buttons[id].state = BUTTON_STATE_RELEASED;
                        buttons[id].press_long = false;
                    }
                } 
                else 
                {
                    buttons[id].state = BUTTON_STATE_PRESSED;
                }
                break;
        }

    }
}
