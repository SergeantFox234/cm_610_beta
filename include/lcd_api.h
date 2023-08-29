#pragma once

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <stats.h>
#include <pins.h>
#include <stdio.h>
#include <esp_log.h>
#include <esp_lcd_panel_interface.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <esp_freertos_hooks.h>
#include <freertos/task.h>
#include <esp_lcd_ili9341.h>
#include <esp_timer.h>
#include <lvgl.h>

// Initialize the display and UI
bool lvgl_init(lv_disp_t* disp, esp_lcd_panel_handle_t panel_handle, lv_disp_drv_t* disp_drv, lv_disp_draw_buf_t* draw_buff, lv_color_t* buf1, lv_color_t* buf2, size_t numPixels);
bool init_lcd(spi_host_device_t host, esp_lcd_panel_handle_t* panel_handle);
void ui_Screen1_screen_init(void);

// Update UI
void set_co2_value(int value);
void set_o2_value(int value);
void set_battery_value(int value);

LV_IMG_DECLARE(ui_img_1409980822);    // assets\CO2Meter-Vertical-Molecule-2023.png (Custom).png
LV_IMG_DECLARE(ui_img_507496363);    // assets\online (Custom).png
//LV_IMG_DECLARE(ui_img_91415062);    // assets\CO2Meter-Vertical-Molecule-2023.png.png