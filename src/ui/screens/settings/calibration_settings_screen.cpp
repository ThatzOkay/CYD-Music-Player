#include "calibration_settings_screen.h"
#include "config.h"
#include "settings_screen.h"
#include <XPT2046_Touchscreen.h>
#include <commons.h>
#include <ui/ui.h>

extern XPT2046_Touchscreen touchscreen;
extern Config config;
extern bool saveConfig();

static const lv_point_t target_points[4] = {
    { 0, 0 },
    {TFT_HOR_RES - 20, 0},
    {TFT_HOR_RES - 20, TFT_VER_RES - 20}, 
    {0, TFT_VER_RES - 20}
};

void CalibrationSettingsScreen::init() {
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101018), 0);

    lv_obj_t *wrapper = lv_obj_create(screen);
    lv_obj_set_style_pad_all(wrapper, 20, 0);
    lv_obj_set_width(wrapper, lv_pct(100));
    lv_obj_set_height(wrapper, lv_pct(100));
    lv_obj_set_flex_flow(wrapper, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_opa(wrapper, LV_OPA_TRANSP, 0);

    lv_obj_t *title = lv_label_create(wrapper);
    lv_label_set_text(title, "Touch Calibration");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);

    info_label = lv_label_create(wrapper);
    lv_label_set_text(info_label, "Touch the red dot (1/4)");
    lv_obj_set_style_text_color(info_label, lv_color_hex(0xCCCCCC), 0);

    dot = lv_obj_create(screen);
    lv_obj_set_size(dot, 20, 20);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(dot, lv_color_hex(0xFF4444), LV_PART_MAIN);
    lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
    lv_obj_set_pos(dot, target_points[0].x, target_points[0].y);
}

void CalibrationSettingsScreen::handle_events() {
    if (step >= 4) return;
    
    if (touchscreen.touched()) {
        TS_Point p = touchscreen.getPoint();
        samples.push_back(p);
        
        while(touchscreen.touched()) {
            delay(10);
        }
        
        next_step();
    }
}

void CalibrationSettingsScreen::next_step() {
    step++;

    if (step >= 4) {
        finish();
        return;
    }

    lv_obj_set_pos(dot, target_points[step].x, target_points[step].y);

    lv_label_set_text_fmt(info_label, "Touch the red dot (%d/4)", step + 1);
}

void CalibrationSettingsScreen::finish() {
    int minX = samples[0].x, maxX = samples[0].x;
    int minY = samples[0].y, maxY = samples[0].y;

    for (auto &p : samples) {
        minX = std::min(minX, (int)p.x);
        maxX = std::max(maxX, (int)p.x);
        minY = std::min(minY, (int)p.y);
        maxY = std::max(maxY, (int)p.y);
    }

    config.calibration.touchMinX = minX;
    config.calibration.touchMaxX = maxX;
    config.calibration.touchMinY = minY;
    config.calibration.touchMaxY = maxY;

    saveConfig();

    lv_obj_clean(screen);

    lv_obj_t *done = lv_label_create(screen);
    lv_label_set_text(done, "Calibration complete!\n(Reboot recommended)");
    lv_obj_center(done);

    lv_timer_create([](lv_timer_t *t){
        switch_screen(new SettingsScreen());
        lv_timer_del(t);
    }, 1200, nullptr);
}
