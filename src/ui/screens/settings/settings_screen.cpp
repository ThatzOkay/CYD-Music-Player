#include "settings_screen.h"
#include <ui/ui.h>
#include "../home_screen.h"
#include "wifi_settings_screen.h"
#include "display_settings_screen.h"
#include "../../components/settings/settings_button.h"
#include "../../components/settings/settings_card.h"

void SettingsScreen::init() {
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101018), 0);

    lv_obj_t *wrapper = lv_obj_create(screen);

    lv_obj_set_style_pad_left(wrapper, 10, 0);
    lv_obj_set_style_pad_right(wrapper, 10, 0);

    lv_obj_set_style_pad_top(wrapper, 25, 0);

    lv_obj_set_flex_flow(wrapper, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(wrapper,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_bottom(wrapper, 0, 0);
    lv_obj_set_style_pad_row(wrapper, 12, 0);

    lv_obj_set_width(wrapper, lv_pct(100));
    lv_obj_set_height(wrapper, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(wrapper, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(wrapper, 0, 0);

    lv_obj_t *title = lv_label_create(wrapper);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);

    lv_obj_t *top_card = ui_settings_card(wrapper);
    
    ui_settings_button(top_card, LV_SYMBOL_LEFT, "Back", []() {
        switch_screen(new HomeScreen());
    });

    lv_obj_t *connections_card = ui_settings_card(wrapper);

    ui_settings_button(connections_card, LV_SYMBOL_WIFI, "WiFi", []() {
        switch_screen(new WifiSettingsScreen());
    });
    ui_settings_button(connections_card, LV_SYMBOL_BLUETOOTH, "Bluetooth", NULL);

    lv_obj_t *display_card = ui_settings_card(wrapper);

    ui_settings_button(display_card, LV_SYMBOL_HOME, "Display", []() {
        switch_screen(new DisplaySettingsScreen());
    });
}

void SettingsScreen::handle_events() {
    // Handle events specific to SettingsScreen if needed
}