#pragma once
#include <pins.h>
#include <esp_vfs_fat.h>
#include <diskio_sdmmc.h>
#include <sys/stat.h>
#include <stdio.h>

bool init_sd(spi_host_device_t host, sdmmc_card_t* sd_card);
bool save_data_to_sd(char* log_line);
void remove_sd(sdmmc_card_t* sd_card);
