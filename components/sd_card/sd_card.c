#include <stdio.h>
#include "esp_log.h"
#include "sd_card.h"
#include "string.h"
#include "dirent.h"

static const char *TAG = "SD_CARD";

esp_err_t sd_read_file(const char *path)
{
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[128];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos)
    {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    return ESP_OK;
}

esp_err_t sd_card_init(sd_card_config_t config, char *mount_path)
{
    esp_err_t ret = ESP_FAIL;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .allocation_unit_size = 16 * 1024,
        .max_files = 5,
    };

    ESP_LOGI(TAG, "Initializing sd card");
    sdmmc_card_t *card;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 4;
    slot_config.clk = config.clk;
    slot_config.cmd = config.cmd;
    slot_config.d0 = config.d0;
    slot_config.d1 = config.d1;
    slot_config.d2 = config.d2;
    slot_config.d3 = config.d3;
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ESP_LOGI(TAG, "Mount filesystem");

    ret = esp_vfs_fat_sdmmc_mount(mount_path, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Filesystem mounted");
    sdmmc_card_print_info(stdout, card);

    /*!< scan files */
    DIR *dir = opendir(mount_path);
    if (!dir)
    {
        return ESP_FAIL;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        ESP_LOGI(TAG, "%s has file:%s", mount_path, entry->d_name);
    }
    return ret;
}