#include "ui.h"
#include "screens/home_screen.h"
#include <WiFi.h>

BaseScreen* current_screen = nullptr;
lv_obj_t* status_bar_wifi_icon = nullptr;
unsigned long last_wifi_check = 0;

void music_player_ui_init() {
    current_screen = new HomeScreen();
    current_screen->init();
    lv_scr_load(current_screen->get_screen());
    create_status_bar(current_screen->get_screen());
}

void music_player_ui_handle_events() {
   current_screen->handle_events();
   update_status_bar();
}

lv_obj_t* create_status_bar(lv_obj_t* parent) {
    lv_obj_t* status_bar = lv_obj_create(parent);
    lv_obj_remove_style_all(status_bar);
    lv_obj_set_size(status_bar, lv_pct(100), 20);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(status_bar, LV_OPA_30, 0);
    lv_obj_set_style_pad_left(status_bar, 10, 0);
    lv_obj_set_style_pad_right(status_bar, 10, 0);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_add_flag(status_bar, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_flag(status_bar, LV_OBJ_FLAG_FLOATING);
    
    lv_obj_set_pos(status_bar, 0, 0);
    
    lv_obj_set_flex_flow(status_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_bar, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    status_bar_wifi_icon = lv_label_create(status_bar);
    lv_label_set_text(status_bar_wifi_icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(status_bar_wifi_icon, lv_color_white(), 0);
    
    lv_obj_add_flag(status_bar_wifi_icon, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_move_foreground(status_bar);
    
    return status_bar;
}

void update_status_bar() {
    if (status_bar_wifi_icon == nullptr) return;
    
    unsigned long now = millis();
    if (now - last_wifi_check > 2000) {
        last_wifi_check = now;
        
        if (WiFi.status() == WL_CONNECTED) {
            lv_obj_clear_flag(status_bar_wifi_icon, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(status_bar_wifi_icon, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

extern void switch_screen(BaseScreen* new_screen) {
    new_screen->init();
    lv_scr_load(new_screen->get_screen());
    
    if (current_screen != nullptr) {
        lv_obj_del(current_screen->get_screen());
        delete current_screen;
    }
    
    current_screen = new_screen;
    
    create_status_bar(new_screen->get_screen());
}