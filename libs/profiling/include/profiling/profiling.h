#pragma once

#include <stdint.h>
#include <stdbool.h>

// по умолчанию профилирование включено
#ifndef PROFILING_ENABLED
#define PROFILING_ENABLED 1
#endif

#if PROFILING_ENABLED
void profiling_start(uint32_t id);
void profiling_stop(uint32_t id);
#else
// профилирование выключено: вызовы ничего не делают
#define profiling_start(id) ((void)0)
#define profiling_stop(id) ((void)0)
#endif

#define STOPWATCHES_COUNT 5

typedef struct {
    const char *name;
    uint32_t count;        // Количество запусков (итераций)
    uint64_t total_us;     // Суммарное время работы в микросекундах
    uint64_t max_us;       // Максимальное время одной итерации
} profiling_result_t;

void profiling_stopwatch_init(uint32_t id, const char *name);

// false — секундомер с таким номером не заведён
bool profiling_get(uint32_t id, profiling_result_t *result);
void profiling_reset(void);