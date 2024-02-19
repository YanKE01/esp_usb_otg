#pragma once

#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

typedef struct
{
    gpio_num_t clk;
    gpio_num_t d0;
    gpio_num_t d1;
    gpio_num_t d2;
    gpio_num_t d3;
    gpio_num_t cmd;
} sd_card_config_t;

esp_err_t sd_card_init(sd_card_config_t config, char *mount_path);
esp_err_t sd_read_file(const char *path);