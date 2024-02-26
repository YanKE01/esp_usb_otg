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
#include "esp_check.h"
#include "hid_device_mouse.h"
#include "hid_device_audio_ctrl.h"
#include "sd_card.h"
#include "st7789.h"
#include "esp_lvgl_port.h"
#include "ui.h"
#include "usb_msc.h"
#include "lcdfont.h"

#define APP_BUTTON (GPIO_NUM_0) // Use BOOT signal by default
// #define USE_HID

static const char *TAG = "example";
lv_disp_t *lvgl_disp = NULL;

lcd_config_t lcd_config = {
    .spi_host_device = SPI3_HOST,
    .dc = GPIO_NUM_4,
    .cs = GPIO_NUM_5,
    .sclk = GPIO_NUM_6,
    .mosi = GPIO_NUM_7,
    .rst = GPIO_NUM_8,
    .lcd_bits_per_pixel = 16,
    .lcd_color_space = LCD_RGB_ELEMENT_ORDER_RGB,
    .lcd_height_res = 240,
    .lcd_vertical_res = 240,
    .lcd_draw_buffer_height = 50,
};

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
    return NULL;
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

esp_err_t lvgl_init()
{
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       /* LVGL task priority */
        .task_stack = 4096,       /* LVGL task stack size */
        .task_affinity = -1,      /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 5      /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGI(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,
        .buffer_size = lcd_config.lcd_height_res * lcd_config.lcd_draw_buffer_height * sizeof(uint16_t),
        .double_buffer = 1,
        .hres = lcd_config.lcd_height_res,
        .vres = lcd_config.lcd_vertical_res,
        .monochrome = false,
        /* Rotation values must be same as used in esp_lcd for initial settings of the screen */
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
        }};

    lvgl_disp = lvgl_port_add_disp(&disp_cfg);
    return ESP_OK;
}

uint16_t POINT_COLOR = swap_hex(rgb565(255, 255, 255)); // 画笔颜色
uint16_t BACK_COLOR = swap_hex(rgb565(0, 0, 0));        // 背景色

void lcd_draw_point(int x, int y)
{
    esp_lcd_panel_draw_bitmap(lcd_panel, x, y, x + 1, y + 1, &POINT_COLOR);
}

/**
 * @brief https://www.zhetao.com/fontarray.html
 * @addindex 高度16，字宽2列，字母A
 *
 */
static const unsigned char bitmap_bytes[] = {
    0x00, 0x00,
    0x00, 0x00,
    0x0e, 0x00,
    0x1e, 0x00,
    0x1f, 0x00,
    0x3f, 0x00,
    0x3b, 0x00,
    0x3b, 0x80,
    0x73, 0x80,
    0x7f, 0xc0,
    0x7f, 0xc0,
    0xe1, 0xc0,
    0xe0, 0xe0,
    0xc0, 0xe0,
    0x00, 0x00,
    0x00, 0x00};

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

    ESP_ERROR_CHECK(lcd_init(lcd_config));
    lcd_fullclean(lcd_panel, lcd_config, rgb565(0, 0, 0));
    uint16_t buffer[16][2 * 8];
    memset(buffer, BACK_COLOR, sizeof(buffer));
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            // 第j列
            uint8_t temp = bitmap_bytes[i * 2 + j];
            for (int k = 0; k < 8; k++)
            {
                if (temp & 0x80)
                {
                    buffer[j * 8 + k][i] = POINT_COLOR;
                    lcd_draw_point(100 + j * 8 + k, 100 + i);
                }
                else
                {
                    buffer[j * 8 + k][i] = BACK_COLOR;
                }
                temp <<= 1;
            }
        }
    }

    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            esp_lcd_panel_draw_bitmap(lcd_panel, 120 + i, 120 + j, 120 + i + 1, 120 + j + 1, &buffer[i][j]);
        }
    }

    for (int i = 0; i < 16; i++)
    {
        esp_lcd_panel_draw_bitmap(lcd_panel, 150 + i, 150, 150 + i + 1, 150 + 16, &buffer[i]);
    }

}