#include "device.h"
#include "pico/unique_id.h"
#include "pico/stdlib.h"
#include "hardware/regs/sysinfo.h"

void device_get_info(device_info_t *info)
{
    if (info == NULL) {
        return;
    }

    info->device_name = DEVICE_NAME;
    info->firmware_version = FIRMWARE_VERSION;
    info->device_project = DEVICE_PROJECT;
    info->device_repo = DEVICE_REPO;
    
    #ifdef PICO_BOARD
    info->board_type = PICO_BOARD;
    #else
    info->board_type = DEVICE_BOARD;
    #endif

    info->sdk_version = PICO_SDK_VERSION_STRING;

    pico_get_unique_board_id_string(info->serial_number, sizeof(info->serial_number));

    volatile uint32_t *chip_id = (uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
    uint32_t id = *chip_id;
    
    info->chip_manufacturer = (id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >> SYSINFO_CHIP_ID_MANUFACTURER_LSB;
    info->chip_part = (id & SYSINFO_CHIP_ID_PART_BITS) >> SYSINFO_CHIP_ID_PART_LSB;
    info->chip_revision = (id & SYSINFO_CHIP_ID_REVISION_BITS) >> SYSINFO_CHIP_ID_REVISION_LSB; // Проверить имя макроса ревизии в Pico SDK
}