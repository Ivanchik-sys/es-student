#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    const char *name;
    uint32_t count;        // Количество запусков (итераций)
    uint64_t total_us;     // Суммарное время работы в микросекундах
    uint64_t max_us;       // Максимальное время одной итерации
} profiling_result_t;

void profiling_stopwatch_init(uint32_t id, const char *name);

void profiling_start(uint32_t id);
void profiling_stop(uint32_t id);

// false — секундомер с таким номером не заведён
bool profiling_get(uint32_t id, profiling_result_t *result);
void profiling_reset(void);