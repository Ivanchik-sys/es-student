#include "api/api.h"
#include "api/api-commands.h"

#include "led-task/led-task.h"
#include "button-task/button-task.h"
#include "pi-task/pi-task.h"
#include "platform/platform.h"
#include "telemetry-task/telemetry-task.h"
#include "block-task/block-task.h"

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
static void cmd_get_voltage(const command_t *command);
static void cmd_get_temp(const command_t *command);
static void cmd_tm_start(const command_t *command);
static void cmd_tm_stop(const command_t *command);
static void cmd_charge_on(const command_t *command);
static void cmd_charge_off(const command_t *command);
static void cmd_timer_blink_start(const command_t *command);
static void cmd_timer_blink_stop(const command_t *command);
static void cmd_cap_start(const command_t *command);
static void cmd_cap_dump(const command_t *command);
static void cmd_dma_cap(const command_t *command);
static void cmd_stream_start(const command_t *command);
static void cmd_stream_stop(const command_t *command);
static void cmd_generate_tone(const command_t *command);
static void cmd_hang(const command_t *command);
static void cmd_wdt_enable(const command_t *command);
static void cmd_reset_reason(const command_t *command);

// период телеметрии по умолчанию
#define TELEMETRY_DEFAULT_PERIOD_MS 100

// период индикатора записи во время потока
#define STREAM_BLINK_PERIOD_MS 250

void api_print_telemetry(const telemetry_record_t *record)
{
    printf("tm: %llu %.3f %.1f\n", record->time_us, record->voltage_v, record->temp_c);
}

void api_send_frame(const stream_header_t *header, const uint8_t *data, uint32_t bytes)
{
    // байты заголовка и отсчётов как они лежат в памяти
    platform_write(header, sizeof(*header));
    platform_write(data, bytes);
}

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
    { "get_voltage", "voltage on GP26, ADC input 0", cmd_get_voltage},
    { "get_temp", "chip temperature, degrees Celsius", cmd_get_temp},
    { "tm_start", "start telemetry: tm_start [<period_ms>]", cmd_tm_start},
    { "tm_stop", "stop telemetry", cmd_tm_stop},
    { "charge_on", "set 3.3V on GP22", cmd_charge_on},
    { "charge_off", "set 0V on GP22", cmd_charge_off},
    { "timer_blink_start", "blink led by timer: timer_blink_start <period_ms>", cmd_timer_blink_start },
    { "timer_blink_stop", "stop timer blink, number of toggles", cmd_timer_blink_stop },
    { "cap_start", "start capture: cap_start <N> <period_us>", cmd_cap_start },
    { "cap_dump", "capture results", cmd_cap_dump },
    { "dma_cap", "return dma cap: dma_cap <N>", cmd_dma_cap},
    { "stream_start", "start stream: stream_start [8\\|16]", cmd_stream_start},
    { "stream_stop", "stop stream", cmd_stream_stop},
    { "generate_tone", "generate tone: 0 — tone: off, else tone: <Hz> Hz", cmd_generate_tone},
    { "hang", "stop superloop", cmd_hang},
    { "wdt_enable", "enable watchdog timer: wdt_enable <timeout_ms>", cmd_wdt_enable},
    { "reset_reason", "show last reset reason and watchdog restarts", cmd_reset_reason},

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

    printf("name: %s\n", info.device_name);
    printf("version: %s\n", info.firmware_version);
    printf("project: %s\n", info.device_project);
    printf("repo: %s\n", info.device_repo);
    printf("board: %s\n", info.board_type);
    printf("serial: %s\n", info.serial_number);
    printf("chip: manufacturer 0x%03lx, part 0x%04lx, revision %lu\n", 
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
    unsigned int time_ms;
    
    pi_state_t state = pi_task_get_state();
    
    switch (state)
    {
        case PI_NOT_STARTED:
            printf("pi: not started\n");
            break;
            
        case PI_RUNNING:
            printf("pi: running\n");
            break;
            
        case PI_READY:
            if (pi_task_get_result(&result, &time_ms))
            {
                // Вывод строго по формату тестов приложения Б
                printf("pi: %f in %u ms\n", result, time_ms);
            }
            break;
    }
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

            printf("%s count %lu mean %.2f us max %llu us\n", 
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
        printf("profiling reset\n");
    #endif
}

static void cmd_led_enable(const command_t *command)
{
    led_task_set_state(LED_STATE_ON);
    
    led_state_t state = led_task_get_state();
    const char *name = led_task_get_state_name(state);
    uint32_t period = led_task_get_period_ms();

    printf("led: %s, period %u ms\n", name, (unsigned int)period);
    return;
}

static void cmd_led_disable(const command_t *command)
{
    led_task_set_state(LED_STATE_OFF);

    led_state_t state = led_task_get_state();
    const char *name = led_task_get_state_name(state);
    uint32_t period = led_task_get_period_ms();
    
    printf("led: %s, period %u ms\n", name, (unsigned int)period);
    return;
}

static void cmd_led_blink(const command_t *command)
{
    led_task_set_state(LED_STATE_BLINK);

    led_state_t state = led_task_get_state();
    const char *name = led_task_get_state_name(state);
    uint32_t period = led_task_get_period_ms();

    printf("led: %s, period %u ms\n", name, (unsigned int)period);
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

    led_state_t state = led_task_get_state();
    const char *name = led_task_get_state_name(state);
    uint32_t period = led_task_get_period_ms();


    printf("led: %s, period %u ms\n", name, (unsigned int)period);
}

static void cmd_button(const command_t *command)
{
    printf("button: %s, presses %u\n", button_task_is_pressed()?"pressed":"released",  button_task_get_press_count());
}

static void cmd_get_voltage(const command_t *command)
{
    if (platform_adc_busy())
    {
        printf("error: adc busy\n");
        return;
    }
    
    printf("voltage: %.3f V\n", platform_voltage_read());
}

static void cmd_get_temp(const command_t *command)
{
    if (platform_adc_busy())
    {
        printf("error: adc busy\n");
        return;
    }
   
    printf("temp: %.1f C\n", platform_temp_read());
}

static void cmd_tm_start(const command_t *command)
{
   if (platform_adc_busy())
    {
        printf("error: adc busy\n");
        return;
    }
   
    uint32_t arg = 100;
    
    if (command->argc > 1)
    {
        printf("error: usage tm_start [<period_ms>]\n");
        return;
    }

    if (command->argc == 1)
    {
        if (!parse_u32(command->argv[0], &arg))
        {
            printf("error: usage tm_start [<period_ms>]\n");
            return;
        }
    
    }
    
    if(telemetry_task_start(arg))
        printf("telemetry: started, period %lu ms\n", telemetry_task_get_period_ms());
    else
        printf("error: period_ms must be at least 10\n");

    return;
}

static void cmd_tm_stop(const command_t *command)
{
    printf("telemetry: stopped\n");
    telemetry_task_stop();
}

static void cmd_charge_on(const command_t *command)
{
    platform_charge_set(true);
    printf("charge: on\n");
}

static void cmd_charge_off(const command_t *command)
{
    platform_charge_set(false);
    printf("charge: off\n");
}

static void cmd_timer_blink_start(const command_t *command)
{
    uint32_t period_ms;
    
    led_task_set_state(LED_STATE_OFF);
    
    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage timer_blink_start <period_ms>\n");
        return;
    }
    
    if(!platform_blink_start(period_ms))
    {
        printf("error: period_ms must be 10..60000\n");
        return;
    };

    printf("timer_blink: started, period %lu ms\n", period_ms);

}

static void cmd_timer_blink_stop(const command_t *command)
{
    uint32_t blink_toggles;
    
    blink_toggles = platform_blink_stop();
    printf("timer_blink: stopped, %lu toggles\n", blink_toggles);
}

static void cmd_cap_start(const command_t *command)
{
    uint32_t count, period_us;

    if (command->argc != 2 || !parse_u32(command->argv[0], &count) || !parse_u32(command->argv[1], &period_us))
    {
        printf("error: usage cap_start <N> <period_us>\n");
        return;
    }

    if (count == 0 || count > PLATFORM_CAPTURE_MAX || period_us < 10)
    {
        printf("error: N must be 1..2000, period_us at least 10\n");
        return;
    }

    if (telemetry_task_get_state() == TELEMETRY_STATE_RUN || !platform_capture_start(count, period_us))
    {
        printf("error: adc busy\n");
        return;
    }

    printf("capture: started, %lu samples, period %lu us\n", count, period_us);

}

static void cmd_cap_dump(const command_t *command)
{
    platform_capture_t cap;
    
    if(!platform_capture_done())
    {
        printf("error: no capture\n");
        return;
    }

    platform_capture_get(&cap);

    printf("capture: %lu samples, period %lu us, interval %lu..%lu us, handler %.1f %%\n", cap.count, cap.period_us, cap.min_interval_us, cap.max_interval_us, cap.handler_percent);
    for (uint32_t i = 0; i < cap.count; i++)
    {
        if (i % 16 == 0)
        {
            printf("cap:");
        }

        printf(" %u", cap.samples[i]);

        if ((i + 1) % 16 == 0 || i + 1 == cap.count)
        {
            printf("\n");
        }
    }
}

static void print_dma_cap(const uint16_t *samples, uint32_t count, uint32_t time_us)
{
    if (samples == NULL || count == 0)
    {
        return;
    }

    uint16_t min_val = UINT16_MAX;
    uint16_t max_val = 0;
    uint64_t sum = 0;

    for (uint32_t i = 0; i < count; i++)
    {
        uint16_t val = samples[i];
        if (val < min_val)
        {
            min_val = val;
        }
        if (val > max_val)
        {
            max_val = val;
        }
        sum += val;
    }

    double mean_val = (double)sum / (double)count;

    uint64_t expected_us = (uint64_t)count * 1000000ULL / PLATFORM_SAMPLE_RATE_HZ;

    printf("dma_cap: %lu samples in %lu us, expected %llu us, min %u max %u mean %.1f\n",
           (unsigned long)count,
           (unsigned long)time_us,
           expected_us,
           min_val,
           max_val,
           mean_val);
}

static void cmd_dma_cap(const command_t *command)
{
    uint32_t count = 0;

    if (command->argc != 1 || !parse_u32(command->argv[0], &count))
    {
        printf("error: usage dma_cap <N>\n");
        return;
    }

    if (count == 0 || count > PLATFORM_BLOCK_MAX)
    {
        printf("error: N must be 1..8192\n");
        return;
    }

    if (telemetry_task_get_state() == TELEMETRY_STATE_RUN || !block_task_start(count, print_dma_cap))
    {
        printf("error: adc busy\n");
        return;
    }
}

void cmd_stream_start(const command_t *command)
{
    uint8_t bits = 16;

    if (command->argc > 1)
    {
        printf("error: usage stream_start [8\\|16]\n");
        return;
    }

    if (command->argc == 1)
    {
        uint32_t parsed_bits = 0;
        if (!parse_u32(command->argv[0], &parsed_bits) || (parsed_bits != 8 && parsed_bits != 16))
        {
            printf("error: usage stream_start [8\\|16]\n");
            return;
        }
        bits = (uint8_t)parsed_bits;
    }

    if (telemetry_task_get_state() == TELEMETRY_STATE_RUN || !stream_task_start(bits))
    {
        printf("error: adc busy\n");
        return;
    }

    // Настройка индикатора записи: мигание с периодом STREAM_BLINK_PERIOD_MS
    // (если в led-task есть функция мигания при выключенной задаче — вызываем её,
    // иначе переводим задачу в LED_STATE_OFF и настраиваем период)
    led_task_set_state(LED_STATE_OFF);
    led_task_set_indicator_ms(STREAM_BLINK_PERIOD_MS);

    // 5. Вывод строгого ответа о запуске
    printf("stream: started, %u Hz, %u bit\n", PLATFORM_SAMPLE_RATE_HZ, bits);
}

void cmd_stream_stop(const command_t *command)
{

    if (stream_task_get_state() == STREAM_STATE_RUN)
    {
        stream_task_stop();
        led_task_set_indicator_ms(0);
    }

    printf("stream: stopped, %lu frames, %lu samples, %lu overruns\n", stream_task_get_frames(), stream_task_get_samples(), stream_task_get_overruns());
}

static void cmd_generate_tone(const command_t *command)
{
    uint32_t freq_hz = 0;

    if (command->argc != 1 || !parse_u32(command->argv[0], &freq_hz))
    {
        printf("error: usage generate_tone <Hz>\n");
        return;
    }

    if (!platform_tone_set(freq_hz))
    {
        printf("error: Hz must be 0 or 20..20000\n");
        return;
    }

    if (freq_hz == 0)
    {
        printf("tone: off\n");
    }
    else
    {
        printf("tone: %lu Hz\n", (unsigned long)freq_hz);
    }
}

static void cmd_hang(const command_t *command)
{
    printf("hang: superloop stopped\n");

    // суперцикл больше не идёт, прерывания приходят как прежде
    while (1)
    {
    }
}

static void cmd_wdt_enable(const command_t *command)
{
    uint32_t timeout_ms = 0;

    if (command->argc != 1 || !parse_u32(command->argv[0], &timeout_ms))
    {
        printf("error: usage wdt_enable <timeout_ms>\n");
        return;
    }

    if (!platform_watchdog_enable(timeout_ms))
    {
        printf("error: timeout_ms must be 1..8388\n");
        return;
    }

    printf("wdt: enabled, timeout %lu ms\n", (unsigned long)timeout_ms);
}

static void cmd_reset_reason(const command_t *command)
{
    uint32_t restarts = 0;

    bool by_wdt = platform_watchdog_caused_reset(&restarts);

    if (by_wdt)
    {
        printf("reset: watchdog, restarts %lu\n", (unsigned long)restarts);
    }
    else
    {
        printf("reset: other, restarts %lu\n", (unsigned long)restarts);
    }
}