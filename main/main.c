/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "hid_device_mouse.h"
#include "hid_device_audio_ctrl.h"
#include "sd_card.h"

#define APP_BUTTON (GPIO_NUM_0) // Use BOOT signal by default
static const char *TAG = "example";
sd_card_config_t sd_card_config = {
    .clk = GPIO_NUM_36,
    .cmd = GPIO_NUM_35,
    .d0 = GPIO_NUM_37,
    .d1 = GPIO_NUM_38,
    .d2 = GPIO_NUM_33,
    .d3 = GPIO_NUM_34,
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    return hid_device_audio_report_descriptor;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
}

void app_main(void)
{
    // Initialize button that will trigger HID reports
    const gpio_config_t boot_button_config = {
        .pin_bit_mask = BIT64(APP_BUTTON),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = true,
        .pull_down_en = false,
    };
    ESP_ERROR_CHECK(gpio_config(&boot_button_config));

    ESP_LOGI(TAG, "USB initialization");
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = hid_device_audio_ctrl_string_descriptor,
        .string_descriptor_count = sizeof(hid_device_audio_ctrl_string_descriptor) / sizeof(hid_device_audio_ctrl_string_descriptor[0]),
        .external_phy = false,
        .configuration_descriptor = hid_device_audio_configuration_descriptor,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_LOGI(TAG, "USB initialization DONE");

    ESP_ERROR_CHECK(sd_card_init(sd_card_config, "/sdcard"));

    while (1)
    {
        if (tud_mounted())
        {
            static bool send_hid_data = true;
            if (send_hid_data)
            {
                ESP_LOGI(TAG, "R:%d", hid_device_audio_test());
            }
            send_hid_data = !gpio_get_level(APP_BUTTON);
        }
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}