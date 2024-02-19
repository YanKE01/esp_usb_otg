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
#include "unistd.h"
#include "sys/stat.h"
#include <fcntl.h>
#include <dirent.h>
#include "jpeg_decoder.h"

#define APP_BUTTON (GPIO_NUM_0) // Use BOOT signal by default
static const char *TAG = "ESP_USB_OTG";
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

uint8_t *file_buffer = NULL; /*!< decode image buffer */
size_t file_buffer_size = 0; /*!< decode image buffer size */

void app_main(void)
{
    ESP_ERROR_CHECK(sd_card_init(sd_card_config, "/sdcard"));

    lcd_init(lcd_config);
    lcd_fullclean(lcd_panel, lcd_config, rgb565(0, 0, 0));

    struct stat st;
    const char *filename = "/sdcard/bilibili.jpg";                                                            /*!< filename */
    if (stat(filename, &st) == 0)
    {
        ESP_LOGI(TAG, "File %s size is %ld\n", filename, st.st_size);
        uint32_t filesize = (uint32_t)st.st_size;                                                             /*!< read image file size */
        char *file_buf = heap_caps_malloc(filesize + 1, MALLOC_CAP_DMA);

        if (file_buf == NULL)
        {
            return;
        }

        int f = open(filename, O_RDONLY);
        if (f > 0)
        {
            read(f, file_buf, filesize);
            ESP_LOGI(TAG, "Decode jpg");
            file_buffer_size = 240 * 240 * sizeof(uint16_t);
            file_buffer = heap_caps_calloc(file_buffer_size, 1, MALLOC_CAP_DEFAULT);                          /*!< create out image buffer */
            esp_jpeg_image_cfg_t jpeg_cfg = {
                .indata = (uint8_t *)file_buf,
                .indata_size = filesize,
                .outbuf = file_buffer,
                .outbuf_size = file_buffer_size,
                .out_format = JPEG_IMAGE_FORMAT_RGB565,
                .out_scale = JPEG_IMAGE_SCALE_1_4,
                .flags = {
                    .swap_color_bytes = 1,
                },
            };

            esp_jpeg_image_output_t outimage;
            esp_jpeg_decode(&jpeg_cfg, &outimage);
            ESP_LOGI(TAG, "%s size: %d x %d", filename, outimage.width, outimage.height);

            esp_lcd_panel_draw_bitmap(lcd_panel, 0, 0, 0 + outimage.height, 0 + outimage.width, file_buffer); /*!< Write to lcd */
            free(file_buffer);
            close(f);
        }
        else
        {
            ESP_LOGI(TAG, "Open %s fail", filename);
        }

        free(file_buf);
    }
    else
    {
        ESP_LOGI(TAG, "Read Size Fail");
        return;
    }
}