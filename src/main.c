#include <sd_api.h>
#include <lcd_api.h>
#include <pins.h>
#include <stats.h>
#include <esp_freertos_hooks.h>
#include <sys/time.h>
#include <lvgl.h>

#define TAG "app_main()"
#define TICK_PERIOD 5 //mS

esp_err_t initL2C();
void initSensorBoard();
void initCommBoard();
void initRelays();
esp_err_t init_spi(const spi_host_device_t host);
void initWifi();
void initBLE();

//Handle UI Updates Using CPU CORE 2
static void lvglTaskHandler() {
    while(1) {
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

//Initialize Hardware and setup RTOS Tasks
void app_main(void) {
    const spi_host_device_t SPI = SPI2_HOST;
    sdmmc_card_t* sd_card = NULL;
    esp_lcd_panel_handle_t panel_handle = NULL;
    bool sd_mounted = false;
    bool lcd_mounted = false;
    bool lvgl_initialized = false;
    BaseType_t taskCreated = pdFAIL;
    esp_err_t esp_return;

    static lv_disp_draw_buf_t disp_buf;
    static lv_disp_drv_t disp_drv;
    static lv_disp_t* disp = NULL;

    // Initialize LTC Bus (Sensor Board and Comm Board)
    esp_return = initL2C();
    if (esp_return == ESP_OK) {
        initSensorBoard();
        initCommBoard();
    }
    
    // Handle Relay States
    initRelays();

    // Initialize SPI Bus (SD and LCD)
    gpio_set_level(SD_CS_PIN, 1);
    gpio_set_level(LCD_CS_PIN, 1);

    esp_return = init_spi(SPI);
    if (esp_return == ESP_OK) {
        sd_mounted = init_sd(SPI, sd_card);
        lcd_mounted = init_lcd(SPI, &panel_handle);
    }

    // ~~ INITIALIZE LVGL PACKAGE FOR LCD ~~ //
    if (lcd_mounted) {
        lv_color_t *buf1 = heap_caps_calloc(NUM_PIXELS, sizeof(lv_color_t), MALLOC_CAP_DMA);
        lv_color_t *buf2 = heap_caps_calloc(NUM_PIXELS, sizeof(lv_color_t), MALLOC_CAP_DMA);
        lvgl_initialized = lvgl_init(disp, panel_handle, &disp_drv, &disp_buf, buf1, buf2, NUM_PIXELS);
    }
    if (lvgl_initialized) {
        taskCreated = xTaskCreatePinnedToCore(lvglTaskHandler, "lvgl timer", 10000, NULL, 4, NULL, CPU_CORE2);

        if (taskCreated != pdPASS) {
            ESP_LOGE(TAG, "FAILED TO CREATE LVGL TIMER TASK\n");
        }
    } 

    // Initialize external communication
    initWifi();
    initBLE();

    // TEST SD CARD
    ESP_LOGI(TAG, "\n\nTESTING SD CARD\n");
    if (sd_mounted) {
        save_data_to_sd("10:42am 8/25 just returned bools\n");
    } else {
        ESP_LOGI(TAG, "There is no SD CARD mounted to write to\n");
    }

    // TEST LCD
    ESP_LOGI(TAG, "\n\nTESTING LCD\n");
    if (lvgl_initialized) {
        ui_Screen1_screen_init();
    } else {
        ESP_LOGE(TAG, "There is no Display mounted to write to\n");
    }

    int i = 0;
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(3000));
        i++;
        //lv_obj_t* label = lv_obj_get_child(lv_scr_act(), 0);
        
        //struct timeval tv;
        //int retVal = gettimeofday(&tv, NULL);
        //if (retVal == 0) {
        //    lv_label_set_text_fmt(label, "%lldmS", ((int64_t)tv.tv_sec * 1000000L + (int64_t)tv.tv_usec) / 1000);
        //}
        set_co2_value(400 + i);
        set_o2_value(21 - i%2);
        set_battery_value(100 - i);

        //Every minute...
        if (i % 6 == 0) {
            if (!sd_mounted) {
                sd_mounted = init_sd(SPI, sd_card);
            }

            if (sd_mounted){
                sd_mounted = save_data_to_sd("Saved every minute...");
                // THIS MEANS IT USED TO BE MOUNTED, BUT WAS UNPLUGGED... UNMOUNT SO A NEW ONCE CAN BE MOUNTED
                if (!sd_mounted) {
                    ESP_LOGE(TAG, "Removing SD...\n");
                    if (sd_card != NULL) {
                        remove_sd(sd_card);
                        sd_card = NULL;
                    }
                    ESP_LOGE(TAG, "...Removed SD\n");
                }
            }
        }
    }
}

esp_err_t initL2C() {
    // TODO IMPLEMENT L2C
    return ESP_OK;
}
void initSensorBoard() {
    // TODO Implement SB
}
void initCommBoard() {
    // TODO Implement CB
}
void initRelays() {
    // TODO Implement Relays
}
esp_err_t init_spi(const spi_host_device_t host) {
    ESP_LOGI(TAG, "Initializing SPI bus...\n");
    const spi_bus_config_t bus_cfg = {
        .mosi_io_num = MOSI_PIN, //11
        .miso_io_num = MISO_PIN, //13
        .sclk_io_num = SCK_PIN, //12
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = HORIZONTAL_RES * VERTICAL_RES * sizeof(uint16_t)
    };
    esp_err_t ret = spi_bus_initialize(host, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "...Initialized SPI Bus\n");
    }
    else {
        ESP_LOGE(TAG, "Failed to initialize SPI bus.\n");
        ESP_LOGE(TAG, "Error: %s\n", esp_err_to_name(ret));
    }
    return ret;
}
void initWifi() {
    // TODO Implement WiFi
}
void initBLE() {
    // TODO Implement BLE
}