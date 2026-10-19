#pragma once

#include <stdint.h>

#define DEVICE_NAME "es-led-module"
#define FIRMWARE_VERSION "1.1.0"

#define DEVICE_PROJECT "231-firmware-template"
#define DEVICE_REPO "https://github.com/Ivanchik-sys/es-student"

#ifndef DEVICE_BOARD
#define DEVICE_BOARD        "unknown"
#endif

typedef struct {
    const char *device_name;
    const char *firmware_version;
    const char *device_project;
    const char *device_repo;
    const char *board_type;
    const char *sdk_version;
    char serial_number[17];
    
    uint32_t chip_manufacturer;
    uint32_t chip_part;
    uint32_t chip_revision;
    
} device_info_t;

void device_get_info(device_info_t *info);
