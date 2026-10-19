#include "profiling.h"
#include "pico/stdlib.h"

typedef enum {
    PROFILING_LOOP,
    PROFILING_COMMAND,
    PROFILING_LED,
    PROFILING_BUTTON,
    PROFILING_PI,
    PROFILING_MAX_STOPWATCHES // Автоматически станет равен 5
} profiling_id_t;

// Внутреннее состояние секундомера прошивки
typedef struct {
    const char *name;
    uint64_t start_us;     // Время старта последнего замера
    uint64_t total_us;     // Накопленное суммарное время
    uint64_t max_us;       // Исторический максимум одной итерации
    uint32_t count;        // Количество запусков
    bool is_initialized;   // Флаг: секундомер заведен
    bool is_running;       // Флаг: секундомер сейчас считает
} stopwatch_internal_t;

// Создаем массив строго под количество наших секундомеров из enum
static stopwatch_internal_t stopwatches[PROFILING_MAX_STOPWATCHES];

void profiling_stopwatch_init(uint32_t id, const char *name)
{
    if (id >= PROFILING_MAX_STOPWATCHES) {
        return;
    }

    stopwatches[id].name = name;
    stopwatches[id].start_us = 0;
    stopwatches[id].total_us = 0;
    stopwatches[id].max_us = 0;
    stopwatches[id].count = 0;
    stopwatches[id].is_initialized = true;
    stopwatches[id].is_running = false;
}

void profiling_start(uint32_t id)
{
    if (id >= PROFILING_MAX_STOPWATCHES || !stopwatches[id].is_initialized) {
        return;
    }

    stopwatches[id].start_us = time_us_64();
    stopwatches[id].is_running = true;
}

void profiling_stop(uint32_t id)
{
    if (id >= PROFILING_MAX_STOPWATCHES || !stopwatches[id].is_running) {
        return;
    }

    uint64_t now_us = time_us_64();
    uint64_t duration = now_us - stopwatches[id].start_us;

    stopwatches[id].total_us += duration;
    stopwatches[id].count++;

    if (duration > stopwatches[id].max_us) {
        stopwatches[id].max_us = duration;
    }

    stopwatches[id].is_running = false;
}

bool profiling_get(uint32_t id, profiling_result_t *result)
{
    if (id >= PROFILING_MAX_STOPWATCHES || !stopwatches[id].is_initialized || result == NULL) {
        return false;
    }

    result->name = stopwatches[id].name;
    result->count = stopwatches[id].count;
    result->total_us = stopwatches[id].total_us;
    result->max_us = stopwatches[id].max_us;

    return true;
}

void profiling_reset(void)
{
    for (uint32_t i = 0; i < PROFILING_MAX_STOPWATCHES; i++) {
        if (stopwatches[i].is_initialized) {
            stopwatches[i].start_us = 0;
            stopwatches[i].total_us = 0;
            stopwatches[i].max_us = 0;
            stopwatches[i].count = 0;
            stopwatches[i].is_running = false;
        }
    }
}