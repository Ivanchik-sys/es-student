#include "api.h"
#include "led-task.h"
#include "button-task.h"
#include "pi-task.h"
#include "log.h"
#include "device.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "profiling.h"


static void cmd_help(const command_t *command);
static void cmd_info(const command_t *command);
static void cmd_uptime(const command_t *command);
static void cmd_pi_start(const command_t *command);
static void cmd_pi(const command_t *command);
static void cmd_main_time_exec(const command_t *command);
static void cmd_main_time_reset(const command_t *command);
static void cmd_led_enable(const command_t *command);
static void cmd_led_disable(const command_t *command);
static void cmd_led_blink(const command_t *command);
static void cmd_led_period(const command_t *command);
static void cmd_button(const command_t *command);

// Число без знака из десятичной записи. false — в записи не только цифры
// или число не помещается в uint32_t.
static bool parse_u32(const char *text, uint32_t *value)
{
    // strtoul приняла бы и знак: из "-1" получилось бы 4294967295
    if (!isdigit((unsigned char)text[0]))
    {
        return false;
    }

    char *end;
    errno = 0;
    unsigned long number = strtoul(text, &end, 10);

    // после числа остались символы или число не поместилось
    if (*end != '\0' || errno == ERANGE || number > UINT32_MAX)
    {
        return false;
    }

    *value = (uint32_t)number;
    return true;
}

const api_command_t api_commands[] = {
    { "help", "list of commands", cmd_help},
    { "info", "device passport", cmd_info },
    { "uptime", "time since reset", cmd_uptime },
    { "pi_start", "start pi calculation: pi_start [terms]", cmd_pi_start },
    { "pi", "pi calculation state", cmd_pi },
    { "main_time_exec", "superloop iteration: average and maximum", cmd_main_time_exec },
    { "main_time_reset", "reset maximum iteration", cmd_main_time_reset },
    { "led_enable", "switch led to ON", cmd_led_enable},
    { "led_disable", "switch led to OFF", cmd_led_disable},
    { "led_blink", "switch led to BLINK", cmd_led_blink},
    { "led_period", "set led period in ms: led_period [period]", cmd_led_period},
    { "button", "button state and press count", cmd_button},
};

const uint api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);

static void cmd_help(const command_t *command)
{
    for (uint i = 0; i < api_command_count; i++) 
    {
        printf("%-16s %s\n", api_commands[i].name, api_commands[i].help);
    }
    return;
}

static void cmd_info(const command_t *command)
{
    device_info();
    return;
}

static void cmd_uptime(const command_t *command)
{
    uint64_t ms = time_us_64() / 1000;

    printf("uptime: %llu ms\n", ms);
    return;
}

// прикидка: за член ряда 4 операции с double, 175 + 110 + 190 + 110 = 585 тактов;
// 1 000 000 членов по 585 тактов при 125 МГц — около 4,7 с
const uint CALC_PI_TERMS = 1000000;

static void cmd_pi_start(const command_t *command)
{
    uint32_t arg = CALC_PI_TERMS;
    
    if (command->argc > 1)
    {
        printf("error: usage pi_start [terms]\n");
        return;
    }

    if (command->argc == 1)
    {
        if (!parse_u32(command->argv[0], &arg))
        {
            printf("error: usage pi_start [terms]\n");
            return;
        }
    
        if (arg < 1)
        {
            printf("error: terms must be greater than 0\n");
            return;
        }
    }
    
    if(pi_task_start(arg))
        printf("pi: started, %u terms\n", arg);
    else
        printf("error: pi is busy\n");

    return;
}

static void cmd_pi(const command_t *command)
{
    double result;
    unsigned int time;
    
    pi_task_get_result(&result, &time);
}

static void cmd_main_time_exec(const command_t *command)
{
    printf("iteration avg %.2f us, max %u us\n", profiling_avg_us(), (unsigned)profiling_max_us());
    return;
}

static void cmd_main_time_reset(const command_t *command)
{
    profiling_reset_max();
    printf("max reset\n");
    return;
}

static void cmd_led_enable(const command_t *command)
{
    led_task_set_state(LED_STATE_ON);
    print_led_state();
    return;
}

static void cmd_led_disable(const command_t *command)
{
    led_task_set_state(LED_STATE_OFF);
    print_led_state();
    return;
}

static void cmd_led_blink(const command_t *command)
{
    led_task_set_state(LED_STATE_BLINK);
    print_led_state();
    return;
}

static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms;

    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage led_period <period_ms>\n");
        return;
    }

    // годится ли такой период, решает задача светодиода
    if (!led_task_set_period_ms(period_ms))
    {
        printf("error: period_ms must be greater than 0\n");
        return;
    }

    // ответ: состояние и период светодиода
    print_led_state();
}

static void cmd_button(const command_t *command)
{
    printf("button: %s, presses: %u\n", button_task_is_pressed()?"pressed":"released",  button_task_get_press_count());
}


void api_handle(const command_t *command)
{
    if(command->truncated)
    {
        printf("error: command longer than 63 characters or 4 arguments\n");
        return;
    }

    for (uint i = 0; i < api_command_count; i++)
    {
        if (strcmp(command->name, api_commands[i].name) == 0)
        {
            api_commands[i].callback(command);
            return;
        }
    }

    printf("error: unknown command: '%s', try help\n", command->name);
}