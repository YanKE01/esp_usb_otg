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

void LCD_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
    uint8_t temp, t, t1;
    uint8_t y0 = y;
    uint8_t *pfont = 0;
    /* 得到字体一个字符对应点阵集所占的字节数 */
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    chr = chr - ' '; /* 得到偏移后的值,因为字库是从空格开始存储的,第一个字符是空格 */

    pfont = (uint8_t *)ascii_1608[chr];

    for (t = 0; t < csize; t++)
    {
        temp = pfont[t];
        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
            {
                //lcd_draw_point(x, y);
            }
            else
            {
                lcd_draw_point(x, y);
            }
            temp <<= 1;
            y++;
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
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

    ESP_ERROR_CHECK(lcd_init(lcd_config));
    lcd_fullclean(lcd_panel, lcd_config, rgb565(0, 0, 0));

    LCD_ShowChar(100, 100, 'a', 16, 1);
    // uint16_t color[12][6];
    // for (int i = 0; i < 12; i++)
    // {
    //     for (int j = 0; j < 6; j++)
    //     {
    //         if (j == 3)
    //         {
    //             color[i][j] = swap_hex(rgb565(0, 0, 0));
    //         }
    //         else
    //         {
    //             color[i][j] = swap_hex(rgb565(255, 255, 255));
    //         }
    //     }
    // }

    // for (int i = 0; i < 12; i++)
    // {
    //     esp_lcd_panel_draw_bitmap(lcd_panel, 20 + i, 50, 20 + i + 1, 50 + 6 + 1, &color[i]);
    // }
}