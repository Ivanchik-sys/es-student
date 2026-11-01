#include "api/api.h"
#include "api/api-commands.h"
#include "led-task/led-task.h"
#include "button-task/button-task.h"
#include "pi-task/pi-task.h"
#include "logging/log.h"
#include "device/device.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "firmware.h"
#include "device/device.h"
#include "profiling/profiling.h"
#include "systime/systime.h"
#include <stdio.h>

static void cmd_help(const command_t *command);
static void cmd_info(const command_t *command);
static void cmd_uptime(const command_t *command);
static void cmd_pi_start(const command_t *command);
static void cmd_pi(const command_t *command);
static void cmd_profiling(const command_t *command);
static void cmd_profiling_reset(const command_t *command);
static void cmd_led_enable(const command_t *command);
static void cmd_led_disable(const command_t *command);
static void cmd_led_blink(const command_t *command);
static void cmd_led_period(const command_t *command);
static void cmd_button(const command_t *command);

const api_command_t api_commands[] = {
    { "help", "list of commands", cmd_help},
    { "info", "device passport", cmd_info },
    { "uptime", "time since reset", cmd_uptime },
    { "pi_start", "start pi calculation: pi_start [terms]", cmd_pi_start },
    { "pi", "pi calculation state", cmd_pi },
    { "profiling", "superloop iteration: average and maximum", cmd_profiling },
    { "profiling_reset", "reset maximum iteration", cmd_profiling_reset },
    { "led_enable", "switch led to ON", cmd_led_enable},
    { "led_disable", "switch led to OFF", cmd_led_disable},
    { "led_blink", "switch led to BLINK", cmd_led_blink},
    { "led_period", "set led period in ms: led_period [period]", cmd_led_period},
    { "button", "button state and press count", cmd_button},
};

const uint32_t api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);


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

static void cmd_help(const command_t *command)
{
    for (uint32_t i = 0; i < api_command_count; i++) 
    {
        printf("%-16s %s\n", api_commands[i].name, api_commands[i].help);
    }
    return;
}

static void cmd_info(const command_t *command)
{
    device_info_t info;
    
    device_get_info(&info);

    info.device_name = FIRMWARE_NAME;
    info.firmware_version = FIRMWARE_VERSION;
    info.device_project = FIRMWARE_PROJECT;
    info.device_repo = FIRMWARE_REPO;

    printf("name:     %s\n", info.device_name);
    printf("version:  %s\n", info.firmware_version);
    printf("project:  %s\n", info.device_project);
    printf("repo:     %s\n", info.device_repo);
    printf("board:    %s\n", info.board_type);
    printf("serial:   %s\n", info.serial_number);
    printf("chip:     manufacturer 0x%03lx, part 0x%04lx, revision %lu\n", 
           info.chip_manufacturer, info.chip_part, info.chip_revision);
    printf("pico-sdk: %s\n", info.sdk_version);

    return;
}

static void cmd_uptime(const command_t *command)
{
    uint64_t ms = systime_us() / 1000;

    printf("uptime: %llu ms\n", ms);
    return;
}

// прикидка: за член ряда 4 операции с double, 175 + 110 + 190 + 110 = 585 тактов;
// 1 000 000 членов по 585 тактов при 125 МГц — около 4,7 с
const uint32_t CALC_PI_TERMS = 1000000;

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
        printf("pi: started, %lu terms\n", arg);
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

static void cmd_profiling(const command_t *command)
{
#if !PROFILING_ENABLED
    printf("profiling is disabled in this build\n");
#else
    profiling_result_t res;

    for (uint32_t i = 0; i < STOPWATCHES_COUNT; i++)
    {
        if (profiling_get(i, &res))
        {
            double mean_us = 0.0;
            if (res.count > 0) {
                mean_us = (double)res.total_us / res.count;
            }

            printf("%-10s count %-10lu mean %-13.2f us  max %llu us\n", 
                   res.name, (unsigned long)res.count, mean_us, res.max_us);
        }
    }
#endif
}

static void cmd_profiling_reset(const command_t *command)
{

    #if !PROFILING_ENABLED
        printf("profiling is disabled in this build\n");
    #else
        profiling_reset(); 
        printf("all stopwatches reset\n");
    #endif
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
