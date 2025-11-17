#include "settings_button.h"

lv_obj_t *ui_settings_button(lv_obj_t *parent,
                             const char *icon,
                             const char *label,
                             void (*on_click)())
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);

    lv_obj_set_width(cont, lv_pct(100));
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont,
                          LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);

    lv_obj_set_style_pad_all(cont, 10, 0);

    if(on_click != NULL) {
        lv_obj_add_event_cb(cont, [](lv_event_t *e) {
            void (*callback)() = (void (*)())lv_event_get_user_data(e);
            if (callback != NULL) {
                callback();
            }
        }, LV_EVENT_CLICKED, (void*)on_click);
    }

    lv_obj_t *button_wrapper = lv_obj_create(cont);
    lv_obj_remove_style_all(button_wrapper);
    lv_obj_set_size(button_wrapper, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(button_wrapper, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(button_wrapper,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(button_wrapper, 8, 0);
    lv_obj_clear_flag(button_wrapper, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *button_icon = lv_obj_create(button_wrapper);
    lv_obj_remove_style_all(button_icon);
    lv_obj_set_size(button_icon, 32, 32);
    lv_obj_set_style_bg_color(button_icon, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_bg_opa(button_icon, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(button_icon, LV_RADIUS_CIRCLE, 0);
    lv_obj_clear_flag(button_icon, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *icon_label = lv_label_create(button_icon);
    lv_label_set_text(icon_label, icon);
    lv_obj_set_style_text_color(icon_label, lv_color_hex(0x000000), 0);
    lv_obj_center(icon_label);
    lv_obj_clear_flag(icon_label, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *back_button_label = lv_label_create(button_wrapper);
    lv_label_set_text(back_button_label, label);
    lv_obj_set_style_text_color(back_button_label, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(back_button_label, &lv_font_montserrat_14, 0);
    lv_obj_clear_flag(back_button_label, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *setting_arrow = lv_label_create(cont);
    lv_label_set_text(setting_arrow, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(setting_arrow, lv_color_hex(0x000000), 0);
    return cont;
}