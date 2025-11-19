#include "display_settings_screen.h"
#include <ui/ui.h>
#include <ui/components/settings/settings_button.h>
#include <ui/components/settings/settings_card.h>
#include "settings_screen.h"
#include "calibration_settings_screen.h"
#include <config.h>

void DisplaySettingsScreen::init()
{
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
    lv_label_set_text(title, "Display Settings");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);

    lv_obj_t *top_card = ui_settings_card(wrapper);

    ui_settings_button(top_card, LV_SYMBOL_LEFT, "Back", []() {
        switch_screen(new SettingsScreen());
    });

    lv_obj_t *brightness_card = ui_settings_card(wrapper);

    lv_obj_t *enable_auto_brightness_wrapper = lv_obj_create(brightness_card);
    lv_obj_remove_style_all(enable_auto_brightness_wrapper);

    lv_obj_set_width(enable_auto_brightness_wrapper, lv_pct(100));
    lv_obj_set_height(enable_auto_brightness_wrapper, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(enable_auto_brightness_wrapper, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(enable_auto_brightness_wrapper,
                          LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_pad_column(enable_auto_brightness_wrapper, 8, 0);
    lv_obj_set_style_pad_bottom(enable_auto_brightness_wrapper, 15, 0);

    lv_obj_t *enable_auto_brightness_label = lv_label_create(enable_auto_brightness_wrapper);
    lv_label_set_text(enable_auto_brightness_label, "Auto Brightness");
    lv_obj_set_style_text_color(enable_auto_brightness_label, lv_color_hex(0x000000), 0);

    lv_obj_t *enable_auto_brightness_switch = lv_switch_create(enable_auto_brightness_wrapper);

    lv_obj_t *brightness_slider = lv_slider_create(brightness_card);
    lv_slider_set_range(brightness_slider, 0, 255);
    lv_slider_set_value(brightness_slider, config.display.brightness, LV_ANIM_OFF);
    lv_obj_set_width(brightness_slider, lv_pct(100));

    lv_obj_set_style_opa(brightness_slider, LV_OPA_50, LV_STATE_DISABLED);

    lv_obj_add_event_cb(brightness_slider, [](lv_event_t *e) {
        lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
        int32_t value = lv_slider_get_value(slider);

        if (value == 0) {
            value = 1;
        }

        config.display.brightness = value;
        ledcWrite(0, value);
        saveConfig();
    }, LV_EVENT_VALUE_CHANGED, NULL);
    
    if (config.display.autoBrightness == true) {
        lv_obj_add_state(enable_auto_brightness_switch, LV_STATE_CHECKED);
        lv_obj_add_state(brightness_slider, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(brightness_slider, LV_STATE_DISABLED);
        lv_slider_set_value(brightness_slider, config.display.brightness, LV_ANIM_OFF);
    }

    lv_obj_add_event_cb(enable_auto_brightness_switch, [](lv_event_t *e) {
        lv_obj_t *sw = (lv_obj_t *)lv_event_get_target(e);
        lv_obj_t *wrapper = lv_obj_get_parent(sw);
        lv_obj_t *card = lv_obj_get_parent(wrapper);
        lv_obj_t *slider = nullptr;

        uint16_t child_count = lv_obj_get_child_cnt(card);
        for (uint16_t i = 0; i < child_count; i++) {
            lv_obj_t *child = lv_obj_get_child(card, i);
            if (lv_obj_check_type(child, &lv_slider_class)) {
                slider = child;
                break;
            }
        }

        if (lv_obj_has_state(sw, LV_STATE_CHECKED)) {
            config.display.autoBrightness = true;
            if (slider) {
                lv_obj_add_state(slider, LV_STATE_DISABLED);
            }
        } else {
            config.display.autoBrightness = false;
            if (slider) {
                lv_obj_clear_state(slider, LV_STATE_DISABLED);
            }
        }

        saveConfig();

    }, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *calibration_card = ui_settings_card(wrapper);

    ui_settings_button(calibration_card, LV_SYMBOL_EDIT, "Calibrate Touchscreen", []() {
        switch_screen(new CalibrationSettingsScreen());
    });
}

void DisplaySettingsScreen::handle_events()
{
    // Handle events specific to DisplaySettingsScreen if needed
}