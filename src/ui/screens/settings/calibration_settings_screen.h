#pragma once

#include "../base_screen.h"
#include <vector>
#include <XPT2046_Touchscreen.h>

class CalibrationSettingsScreen : public BaseScreen {
private:
    lv_obj_t *dot;
    lv_obj_t *info_label;
    int step = 0;
    std::vector<TS_Point> samples;

public:
    void init() override;
    void handle_events() override;

    void next_step();
    void finish();
};