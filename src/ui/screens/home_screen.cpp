#include "home_screen.h"
#include "../components/icon_button.h"
#include "../ui.h"
#include "settings/settings_screen.h"
#include <Arduino.h>

void on_songs_click() {
    Serial.println("Songs button clicked!");
    // TODO: Navigate to songs screen
}

void on_albums_click() {
    Serial.println("Albums button clicked!");
    // TODO: Navigate to albums screen
}

void on_shuffle_click() {
    Serial.println("Shuffle All button clicked!");
    // TODO: Start shuffle play
}

void on_radio_click() {
    Serial.println("Radio button clicked!");
    // TODO: Navigate to radio screen
}

void on_settings_click() {
    switch_screen(new SettingsScreen());
}

void HomeScreen::init() {
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101018), 0);
    lv_obj_set_style_pad_bottom(screen, 0, 0);

    lv_obj_t *wrapper = lv_obj_create(screen);

    lv_obj_set_style_pad_top(wrapper, 25, 0);

    lv_obj_set_style_pad_left(wrapper, 10, 0);
    lv_obj_set_style_pad_right(wrapper, 10, 0);

    lv_obj_set_flex_flow(wrapper, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(wrapper,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(wrapper, 12, 0);
    lv_obj_set_style_pad_column(wrapper, 12, 0);

    lv_obj_set_width(wrapper, lv_pct(100));
    lv_obj_set_height(wrapper, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(wrapper, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(wrapper, 0, 0);

    ui_icon_button(wrapper, LV_SYMBOL_AUDIO, "Songs", lv_color_hex(0x4BA3FF), on_songs_click);
    ui_icon_button(wrapper, LV_SYMBOL_LIST, "Albums", lv_color_hex(0xFF8C5A), on_albums_click);
    ui_icon_button(wrapper, LV_SYMBOL_SHUFFLE, "Shuffle All", lv_color_hex(0x67D67F), on_shuffle_click);
    ui_icon_button(wrapper, LV_SYMBOL_WIFI, "Radio", lv_color_hex(0xFF6B9D), on_radio_click);
    ui_icon_button(wrapper, LV_SYMBOL_SETTINGS, "Settings", lv_color_hex(0x9B59B6), on_settings_click);
}

void HomeScreen::handle_events() {
    // Handle events specific to HomeScreen if needed
}
