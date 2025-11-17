#pragma once
#include <lvgl.h>

lv_obj_t* ui_settings_button(lv_obj_t *parent,
                             const char *icon,
                             const char *label,
                             void (*on_click)());