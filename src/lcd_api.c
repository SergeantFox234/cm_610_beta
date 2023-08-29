#include <lcd_api.h>

#define TAG "LCD"

#define TICK_PERIOD 5 //mS

static lv_obj_t * ui_Screen1;
static lv_obj_t * ui_Header;
static lv_obj_t * ui_Logo;
static lv_obj_t * ui_Connected;
static lv_obj_t * ui_Battery_Label;
static lv_obj_t * ui_CO2_Group;
static lv_obj_t * ui_CO2_Dial;
static lv_obj_t * ui_CO2_Value;
static lv_obj_t * ui_CO2_Label;
static lv_obj_t * ui_O2_Group;
static lv_obj_t * ui_Oxy_Dial;
static lv_obj_t * ui_Oxy_Value;
static lv_obj_t * ui_Oxy_Label;

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
    esp_lcd_panel_handle_t panel_handle = disp_drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    // copy a buffer's content to a specific area of the display
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_p));

    //IMPORTANT!!!
    // Inform the graphics library that you are ready with the flushing
    lv_disp_flush_ready(disp_drv);
}

bool init_lcd(spi_host_device_t host, esp_lcd_panel_handle_t* panel_handle) {
    //Allocate an LCD IO device handle
    ESP_LOGE(TAG, "Initializing LCD...");

    esp_lcd_panel_io_handle_t io_handle = NULL;
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = DC_PIN,
        .cs_gpio_num = LCD_CS_PIN,
        .pclk_hz = 6350000,
        .lcd_cmd_bits = 8, //8 for 4 pin and 9 for 3 pin
        .lcd_param_bits = 8, //8 for 4 pin and 9 for 3 pin
        .spi_mode = 0,
        .trans_queue_depth = 20,
    };

    esp_err_t attachErr = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t) host, &io_config, &io_handle);
    if (attachErr != ESP_OK) {
        ESP_LOGE(TAG, "Failed to attach display to SPI Bus.\n");
        ESP_LOGE(TAG, "The Error Code Returned Was %s\n", esp_err_to_name(attachErr));
        return false;
    }

    ESP_LOGE(TAG, "...Initialized LCD!!!\n");

    //Allocate an LCD IO Panel Handle
    ESP_LOGE(TAG, "Initializing LCD IO Panel...");

    //esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = RST_PIN,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16,
        .flags = {.reset_active_high = 0}
    };
    esp_err_t newPanelErr = esp_lcd_new_panel_ili9341(io_handle, &panel_config, panel_handle);
    
    if (newPanelErr != ESP_OK) {
        ESP_LOGE(TAG, "Failed to attach new display pannel to SPI Bus.\n");
        ESP_LOGE(TAG, "The Error Code Returned Was %s\n", esp_err_to_name(newPanelErr));
        return false;
    }
    ESP_LOGE(TAG, "...Initialized LCD IO Panel!!!\n");

    ESP_LOGI(TAG, "Turn off LCD backlight");
    const gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << BLK_PIN
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    //Init the LCD Panel
    ESP_ERROR_CHECK(esp_lcd_panel_reset(*panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(*panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(*panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(*panel_handle, false, true));

    ESP_LOGI(TAG, "Turn on LCD backlight\n");
    gpio_set_level(BLK_PIN, 1);

    //User can flush pre-defined pattern to the screen before we turn on the screen or backlight
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(*panel_handle, true));

    return true;
}

bool lvgl_init(lv_disp_t* disp, esp_lcd_panel_handle_t panel_handle, lv_disp_drv_t* disp_drv, lv_disp_draw_buf_t* draw_buff, lv_color_t* buf1, lv_color_t* buf2, size_t numPixels) {
    //Initialize the LVGL Package
    lv_init();

    //Initialize the display buffer
    lv_disp_draw_buf_init(draw_buff, buf1, buf2, numPixels);

    //Basic initialization
    lv_disp_drv_init(disp_drv);
    disp_drv->hor_res = HORIZONTAL_RES;
    disp_drv->ver_res = VERTICAL_RES;
    disp_drv->flush_cb = disp_flush;
    disp_drv->user_data = panel_handle;
    disp_drv->antialiasing = true;

    //Set a display buffer
    disp_drv->draw_buf = draw_buff;
    
    //Finally register the driver
    disp = lv_disp_drv_register(disp_drv);
    if (disp == NULL) {
        ESP_LOGE(TAG, "ERROR REGISTERING DISPLAY WITH LVGL!!!~~~");
        return false;
    }

    ESP_LOGI(TAG, "DISPLAY CREATED:--> W %u, H %u", lv_disp_get_hor_res(disp), lv_disp_get_ver_res(disp));

   return true;
}

void ui_Screen1_screen_init(void) {
    ui_Screen1 = lv_scr_act();
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_Screen1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    // ~~ HEADER ~~ //
    ui_Header = lv_obj_create(ui_Screen1);
    lv_obj_set_height(ui_Header, 60);
    lv_obj_set_width(ui_Header, lv_pct(100));
    lv_obj_set_align(ui_Header, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(ui_Header, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(ui_Header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Header, lv_color_hex(0x40AA07), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_Header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Logo = lv_img_create(ui_Header);
    lv_img_set_src(ui_Logo, &ui_img_1409980822);
    lv_obj_set_width(ui_Logo, LV_SIZE_CONTENT);   /// 40
    lv_obj_set_height(ui_Logo, LV_SIZE_CONTENT);    /// 40
    lv_obj_set_align(ui_Logo, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_Logo, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_Logo, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    
    ui_Connected = lv_img_create(ui_Header);
    lv_img_set_src(ui_Connected, &ui_img_507496363);
    lv_obj_set_width(ui_Connected, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Connected, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_Connected, LV_ALIGN_LEFT_MID);
    lv_obj_add_flag(ui_Connected, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_Connected, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    
    ui_Battery_Label = lv_label_create(ui_Header);
    lv_obj_set_width(ui_Battery_Label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Battery_Label, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_Battery_Label, LV_ALIGN_RIGHT_MID);
    lv_label_set_text(ui_Battery_Label, "100%");
    lv_obj_set_style_text_color(ui_Battery_Label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_Battery_Label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ~~ CO2 Dial ~~ //
    ui_CO2_Group = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_CO2_Group, 150);
    lv_obj_set_height(ui_CO2_Group, 150);
    lv_obj_set_x(ui_CO2_Group, 7);
    lv_obj_set_y(ui_CO2_Group, 30);
    lv_obj_set_align(ui_CO2_Group, LV_ALIGN_LEFT_MID);
    lv_obj_clear_flag(ui_CO2_Group, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_CO2_Group, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_CO2_Dial = lv_arc_create(ui_CO2_Group);
    lv_obj_set_width(ui_CO2_Dial, 150);
    lv_obj_set_height(ui_CO2_Dial, 150);
    lv_obj_set_x(ui_CO2_Dial, 1);
    lv_obj_set_y(ui_CO2_Dial, 0);
    lv_obj_set_align(ui_CO2_Dial, LV_ALIGN_CENTER);
    lv_arc_set_range(ui_CO2_Dial, 0, 5000);
    lv_arc_set_bg_angles(ui_CO2_Dial, 0, 360);
    lv_obj_set_style_pad_left(ui_CO2_Dial, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_CO2_Dial, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_CO2_Dial, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_CO2_Dial, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(ui_CO2_Dial, lv_color_hex(0x53C801), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_CO2_Dial, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_CO2_Dial, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_CO2_Value = lv_label_create(ui_CO2_Dial);
    lv_obj_set_width(ui_CO2_Value, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_CO2_Value, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_CO2_Value, 0);
    lv_obj_set_y(ui_CO2_Value, 15);
    lv_obj_set_align(ui_CO2_Value, LV_ALIGN_CENTER);
    set_co2_value(400);
    lv_obj_set_style_text_font(ui_CO2_Value, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_CO2_Label = lv_label_create(ui_CO2_Group);
    lv_obj_set_width(ui_CO2_Label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_CO2_Label, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_CO2_Label, 0);
    lv_obj_set_y(ui_CO2_Label, 25);
    lv_obj_set_align(ui_CO2_Label, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_CO2_Label, "CO2");
    lv_obj_set_style_text_font(ui_CO2_Label, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    // ~~ O2 Dial ~~ //
    ui_O2_Group = lv_obj_create(ui_Screen1);
    lv_obj_set_width(ui_O2_Group, 150);
    lv_obj_set_height(ui_O2_Group, 150);
    lv_obj_set_x(ui_O2_Group, -7);
    lv_obj_set_y(ui_O2_Group, 30);
    lv_obj_set_align(ui_O2_Group, LV_ALIGN_RIGHT_MID);
    lv_obj_clear_flag(ui_O2_Group, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_O2_Group, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Oxy_Dial = lv_arc_create(ui_O2_Group);
    lv_obj_set_width(ui_Oxy_Dial, 150);
    lv_obj_set_height(ui_Oxy_Dial, 150);
    lv_obj_set_align(ui_Oxy_Dial, LV_ALIGN_CENTER);
    lv_arc_set_range(ui_Oxy_Dial, 18, 25);
    lv_arc_set_bg_angles(ui_Oxy_Dial, 0, 360);
    lv_obj_set_style_pad_left(ui_Oxy_Dial, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui_Oxy_Dial, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui_Oxy_Dial, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui_Oxy_Dial, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(ui_Oxy_Dial, lv_color_hex(0x0098CE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_Oxy_Dial, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_Oxy_Dial, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Oxy_Dial, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_Oxy_Value = lv_label_create(ui_Oxy_Dial);
    lv_obj_set_width(ui_Oxy_Value, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Oxy_Value, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Oxy_Value, 0);
    lv_obj_set_y(ui_Oxy_Value, 15);
    lv_obj_set_align(ui_Oxy_Value, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Oxy_Value, "21%");
    lv_obj_set_style_text_font(ui_Oxy_Value, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Oxy_Label = lv_label_create(ui_O2_Group);
    lv_obj_set_width(ui_Oxy_Label, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Oxy_Label, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Oxy_Label, 0);
    lv_obj_set_y(ui_Oxy_Label, 25);
    lv_obj_set_align(ui_Oxy_Label, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_Oxy_Label, "O2");
    lv_obj_set_style_text_font(ui_Oxy_Label, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
}

// USE THESE FUNCTIONS TO UPDATE DISPLAY FROM SENSOR VALUES
void set_co2_value(int value) {
    if (value < 0) {
        value = 0;
    }
    
    lv_label_set_text_fmt(ui_CO2_Value, "%uppm", value);
}
void set_o2_value(int value) {
    if (value < 0) {
        value = 0;
    }
    else if (value > 100) {
        value = 100;
    }
    lv_label_set_text_fmt(ui_Oxy_Value, "%u%%", value);
}
void set_battery_value(int value) {
    if (value < 0) {
        value = 0;
    }
    else if (value > 100) {
        value = 100;
    }
    lv_label_set_text_fmt(ui_Battery_Label, "%u%%", value);
}