#pragma once
#include <lvgl.h>

lv_obj_t* ui_icon_button(lv_obj_t *parent,
                         const char *icon,
                         const char *label,
                         lv_color_t color,
                        void (*on_click)());