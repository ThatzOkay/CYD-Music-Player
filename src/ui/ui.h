#pragma once
#include <lvgl.h>

void music_player_ui_init();

void music_player_ui_handle_events();

extern void switch_screen(class BaseScreen* new_screen);

extern lv_obj_t* create_status_bar(lv_obj_t* parent);
extern void update_status_bar();