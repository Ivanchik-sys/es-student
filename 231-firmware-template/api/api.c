#include "api.h"
#include "led.h"
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

//#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

//extern const api_command_t api_commands[];

static void cmd_help(const command_t *command);
static void cmd_info(const command_t *command);
static void cmd_uptime(const command_t *command);
static void cmd_calc_pi(const command_t *command);
static void cmd_main_time_exec(const command_t *command);
static void cmd_main_time_reset(const command_t *command);

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
    { "calc_pi", "calculate pi: calc_pi [terms]", cmd_calc_pi },
    { "main_time_exec", "superloop iteration: average and maximum", cmd_main_time_exec },
    { "main_time_reset", "reset maximum iteration", cmd_main_time_reset },
};

const uint api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);

static void cmd_help(const command_t *command)
{
    for (uint i = 0; i < api_command_count; i++) 
    {
        printf("%-16s %s\n", api_commands[i].name, api_commands[i].help);
    }
}

static void cmd_info(const command_t *command)
{
    device_info();
}

static void cmd_uptime(const command_t *command)
{
    uint64_t ms = time_us_64() / 1000;

    printf("uptime: %llu ms\n", ms);
}

// прикидка: за член ряда 4 операции с double, 175 + 110 + 190 + 110 = 585 тактов;
// 1 000 000 членов по 585 тактов при 125 МГц — около 4,7 с
const uint CALC_PI_TERMS = 1000000;

double calc_pi(uint terms)
{
    double sum = 0.0;
    double sign = 1.0;

    for (int k = 0; k < terms; k++) {
        sum += sign / (2.0 * k + 1.0);
        sign = -sign;
    }
    return 4.0 * sum;
}

volatile double pi_result;

static void cmd_calc_pi(const command_t *command)
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
    
    uint64_t start_us = time_us_64();
    pi_result = calc_pi(arg);
    uint64_t spent_us = time_us_64() - start_us;

    printf("pi: %.8f\n", pi_result);
    printf("time: %llu ms\n", spent_us / 1000);
}

static void cmd_main_time_exec(const command_t *command)
{
    printf("iteration avg %.2f us, max %u us\n", profiling_avg_us(), (unsigned)profiling_max_us());
}

static void cmd_main_time_reset(const command_t *command)
{
    profiling_reset_max();
    printf("max reset\n");
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