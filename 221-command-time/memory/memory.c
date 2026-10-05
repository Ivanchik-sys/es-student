#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "device.h"
#include "memory.h"
#include "command.h"
#include "led.h"
#include "hardware/regs/addressmap.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;

uint32_t data_variable = 100;
uint32_t bss_variable;

#define VECTOR_TABLE 0x10000100

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}

int main(void);

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

void fw_info(void)
{
    data_variable = data_variable + 1;
    bss_variable = bss_variable + 1;

    uint32_t stack_variable = 1946;
    uint32_t *heap_variable = malloc(sizeof(uint32_t));

    if (heap_variable != NULL)
    {
        *heap_variable = 1951;
    }

    uint16_t *main_code = (uint16_t *)((uintptr_t)main & ~1u);
    uintptr_t main_entry = (uintptr_t)main_code | 1u;
    uint16_t *fw_info_code = (uint16_t *)((uintptr_t)fw_info & ~1u);
    uintptr_t fw_info_entry = (uintptr_t)fw_info_code | 1u;

    printf("object          address     value\n");
    printf("%-15s 0x%08x  0x%x\n", "main", main_entry, *main_code);
    printf("%-15s 0x%08x  0x%x\n", "fw_info", fw_info_entry, *fw_info_code);
    
    printf("%-15s 0x%08x\n", "commands", &commands);

    for(int i = 0; i < command_count; i++)
    {
        uint16_t *command_code = (uint16_t *)((uintptr_t)commands[i].handler & ~1u);
        printf(" - %-13s 0x%08x\n", commands[i].name, (uintptr_t)command_code);
    }

    printf("%-15s 0x%08x  %s\n", "DEVICE_PROJECT", &DEVICE_PROJECT, DEVICE_PROJECT);
    printf("%-15s 0x%08x  %s\n", "DEVICE_BOARD", &DEVICE_BOARD, DEVICE_BOARD);
    printf("%-15s 0x%08x  %u\n", "data_variable", &data_variable, data_variable);
    printf("%-15s 0x%08x  %u\n", "bss_variable", &bss_variable, bss_variable);
    printf("%-15s 0x%08x  %u\n", "stack_variable", &stack_variable, stack_variable);
    printf("%-15s 0x%08x  %u\n", "heap_variable", heap_variable, *heap_variable);

    free(heap_variable);
}

void boot_info(void)
{
    const uint32_t *vectors = (const uint32_t *)VECTOR_TABLE;

    uint32_t stack_top = vectors[0];
    uint32_t reset_handler = vectors[1];
    uint32_t reset_even = reset_handler & ~1u;

    volatile uint32_t *gpio_in = (uint32_t *)(SIO_BASE + SIO_GPIO_IN_OFFSET);
    uint32_t level = (*gpio_in >> led_pin()) & 1u;
    
    printf("%-14s 0x%x\n", "vector table", vectors);
    printf("%-14s 0x%x\n", "  stack top", stack_top);
    printf("%-14s 0x%x\n", "  reset", reset_handler);
    printf("%-14s 0x%x\n", "  reset (even)", reset_even);

    printf("%-14s 0x%x\n", "gpio in", gpio_in);
    printf("%-14s %u\n", "  led bit", level);
    printf("%-14s %u\n", "  gpio_get", gpio_get(led_pin()));
    
}
