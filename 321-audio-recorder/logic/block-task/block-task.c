#include "block-task/block-task.h"
#include "platform/platform.h"
#include <stddef.h>

static uint32_t target_count = 0;
static block_task_callback_t current_callback = NULL;

bool block_task_start(uint32_t count, block_task_callback_t on_block)
{
    if (current_callback != NULL || on_block == NULL)
    {
        return false;
    }

    if (!platform_block_start(count))
    {
        return false;
    }

    target_count = count;
    current_callback = on_block;
    return true;
}

void block_task_handle(void)
{
    if (current_callback == NULL || !platform_block_done())
    {
        return;
    }

    block_task_callback_t cb = current_callback;
    uint32_t count = target_count;
    current_callback = NULL;
    target_count = 0;

    cb(platform_block_samples(), count,  platform_block_time_us());
}