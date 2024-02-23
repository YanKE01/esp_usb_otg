#pragma once

#include "tinyusb.h"
#include "esp_err.h"
#include "sdmmc_cmd.h"

extern const char *usb_msc_string_descriptor[];
extern tusb_desc_device_t usb_msc_device_descriptor;
extern const uint8_t usb_msc_configuration[];

/**
 * @brief init usb mass storage
 * 
 * @param card 
 * @return esp_err_t 
 */
esp_err_t usb_msc_init(sdmmc_card_t **card);
