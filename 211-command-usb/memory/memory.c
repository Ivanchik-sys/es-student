#include <stdio.h>
#include <stdint.h>
#include "memory.h"
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}

void mem_info(void)
{
    printf("area       start      end        size\n");
    row("flash", XIP_BASE, XIP_BASE + PICO_FLASH_SIZE_BYTES);
    row("sram", SRAM_BASE, SRAM_BASE + 270336);
    row("rom", ROM_BASE, ROM_BASE + 16384);
    
    uintptr_t flash_start = (uintptr_t)&__flash_binary_start;
    uintptr_t flash_end   = (uintptr_t)&__flash_binary_end;
    uintptr_t boot2_start = (uintptr_t)&__boot2_start__;
    uintptr_t boot2_end   = (uintptr_t)&__boot2_end__;
    uintptr_t etext       = (uintptr_t)&__etext;
    uintptr_t data_start  = (uintptr_t)&__data_start__;
    uintptr_t data_end    = (uintptr_t)&__data_end__;
    uintptr_t bss_start   = (uintptr_t)&__bss_start__;
    uintptr_t bss_end     = (uintptr_t)&__bss_end__;
    uintptr_t heap_limit  = (uintptr_t)&__HeapLimit;
    uintptr_t stack_bot   = (uintptr_t)&__StackBottom;
    uintptr_t stack_top   = (uintptr_t)&__StackTop;

    row("image", flash_start, flash_end);
    row("free", flash_end, XIP_BASE + PICO_FLASH_SIZE_BYTES);
    row("boot2", boot2_start, boot2_end);
    row("text", boot2_end, etext);

    row("data flash", etext, etext + (data_end - data_start));
    row("data ram", data_start, data_end);
    row("bss", bss_start, bss_end);
    row("heap", bss_end, heap_limit);
    row("stack", stack_bot, stack_top);

    printf("\n\ntotal\n");
    
    printf("  flash image %8u = ", (unsigned)(flash_end - flash_start));
    printf("boot2 %u + ", (unsigned)(boot2_end - boot2_start));
    printf("text %u + ", (unsigned)(etext - boot2_end));
    printf("data %u\n", (unsigned)(data_end - data_start));
    printf("  flash free %9u of %u\n", (unsigned)(XIP_BASE + PICO_FLASH_SIZE_BYTES - flash_end), (unsigned)PICO_FLASH_SIZE_BYTES);
    printf("  ram used %11u = ", (unsigned)(data_end - data_start + bss_end - bss_start));
    printf("data %u + ", (unsigned)(data_end - data_start));
    printf("bss %u\n", (unsigned)(bss_end - bss_start));
    printf("  ram free %11u for heap ", (unsigned)(heap_limit - bss_end));
    printf("and %u for stack\n", (unsigned)(stack_top - stack_bot));
}