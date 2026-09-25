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
    

    row("image", (uintptr_t)&__flash_binary_start, (uintptr_t)&__flash_binary_end);
    row("free", (uintptr_t)&__flash_binary_end, XIP_BASE + PICO_FLASH_SIZE_BYTES);
    row("boot2", (uintptr_t)&__boot2_start__, (uintptr_t)&__boot2_end__);
    row("text", (uintptr_t)&__boot2_end__, (uintptr_t)&__etext);

    row("data flash", (uintptr_t)&__etext, (uintptr_t)&__etext + ((uintptr_t)&__data_end__ - (uintptr_t)&__data_start__));
    row("data ram", (uintptr_t)&__data_start__, (uintptr_t)&__data_end__);
    row("bss", (uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__);
    row("heap", (uintptr_t)&__bss_end__, (uintptr_t)&__HeapLimit);
    row("stack", (uintptr_t)&__StackBottom, (uintptr_t)&__StackTop);

    printf("\n\ntotal\n");
    printf("  flash image %8u = ", (unsigned)(&__flash_binary_end - &__flash_binary_start));
    printf("boot2 %u + ", (unsigned)(&__boot2_end__ - &__boot2_start__));
    printf("text %u + ", (unsigned)(&__etext - &__boot2_end__));
    printf("data %u\n", (unsigned)(&__data_end__ - &__data_start__));
    printf("  flash free %9u of %u\n", (unsigned)(XIP_BASE + PICO_FLASH_SIZE_BYTES - (unsigned)&__flash_binary_end), (unsigned)PICO_FLASH_SIZE_BYTES);
    printf("  ram used %11u = ", (unsigned)(&__data_end__ - &__data_start__ + &__bss_end__ - &__bss_start__));
    printf("data %u + ", (unsigned)(&__data_end__ - &__data_start__));
    printf("bss %u\n", (unsigned)(&__bss_end__ - &__bss_start__));
    printf("  ram free %11u for heap ", (unsigned)(&__HeapLimit - &__bss_end__));
    printf("and %u for stack\n", (unsigned)(&__StackTop - &__StackBottom));
}