#include "icon_button.h"

lv_obj_t* ui_icon_button(lv_obj_t *parent,
                         const char *icon,
                         const char *label,
                         lv_color_t color,
                        void (*on_click)())
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont, 8, 0);

    lv_obj_t *btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 70, 70);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn, color, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    
    if (on_click != NULL) {
        lv_obj_add_event_cb(btn, [](lv_event_t *e) {
            void (*callback)() = (void (*)())lv_event_get_user_data(e);
            if (callback != NULL) {
                callback();
            }
        }, LV_EVENT_CLICKED, (void*)on_click);
    }

    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_t *icon_label = lv_label_create(btn);
    lv_label_set_text(icon_label, icon);
    lv_obj_set_style_text_color(icon_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_28, 0);
    lv_obj_clear_flag(icon_label, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *text_label = lv_label_create(cont);
    lv_label_set_text(text_label, label);
    lv_obj_set_style_text_color(text_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(text_label, &lv_font_montserrat_14, 0);
    lv_obj_clear_flag(text_label, LV_OBJ_FLAG_CLICKABLE);

    return cont;
}
