#include "sd_api.h"

#define TAG "SDCARD"
#define PATH "/sdcard"

bool init_sd(spi_host_device_t host, sdmmc_card_t* sd_card) {
    const sdmmc_host_t host_config = {
        .flags = SDMMC_HOST_FLAG_SPI | SDMMC_HOST_FLAG_DEINIT_ARG,
        .slot = host,
        .max_freq_khz = SDMMC_FREQ_DEFAULT,
        .io_voltage = 3.3f,
        .init = &sdspi_host_init,
        .set_bus_width = NULL,
        .get_bus_width = NULL,
        .set_bus_ddr_mode = NULL,
        .set_card_clk = &sdspi_host_set_card_clk,
        .do_transaction = &sdspi_host_do_transaction,
        .deinit_p = &sdspi_host_remove_device,
        .io_int_enable = &sdspi_host_io_int_enable,
        .io_int_wait = &sdspi_host_io_int_wait,
        .command_timeout_ms = 10000,
    };

    const sdspi_device_config_t slot_config = {
        .host_id   = host,
        .gpio_cs   = SD_CS_PIN,
        .gpio_cd   = SDSPI_SLOT_NO_CD,
        .gpio_wp   = SDSPI_SLOT_NO_WP,
        .gpio_int  = SDSPI_SLOT_NO_INT,
    };

    const esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
        .disk_status_check_enable = true
    };

    // HERE IT IS BABY
    ESP_LOGE(TAG, "Mounting SD Card...\n");
    const esp_err_t mountErr = esp_vfs_fat_sdspi_mount(PATH, &host_config, &slot_config, &mount_config, &sd_card);
    if (mountErr != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount filesystem.\n");
        ESP_LOGE(TAG, "The Error Code Returned Was %s\n", esp_err_to_name(mountErr));
        return false;
    }
    ESP_LOGE(TAG, "...Mounted SD Card\n");
    return true;
}
void remove_sd(sdmmc_card_t* sd_card) {
    ESP_LOGD(TAG, "Unmounting SD Card...\n");

    const esp_err_t unmountErr = esp_vfs_fat_sdcard_unmount(PATH, sd_card);
    if (unmountErr != ESP_OK) {
        ESP_LOGE(TAG, "Error: %s\n", esp_err_to_name(unmountErr));
        return;
    }

    ESP_LOGD(TAG, "...Unmounted SD Card\n");
}
bool save_data_to_sd(char* log_line) {
    ESP_LOGI(TAG, "Writing to file...\n");

    FILE* f = fopen(PATH"/datalog.txt", "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing\n");
        return false;
    }
    fprintf(f, log_line);
    fclose(f);

    ESP_LOGI(TAG, "Wrote to file!!\n");
    ESP_LOGI(TAG, "Success in writing to SD card\n");
    return true;
};