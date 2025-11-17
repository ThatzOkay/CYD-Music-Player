#pragma once
#include <lvgl.h>
#include "../base_screen.h"

class SettingsScreen : public BaseScreen {
public:
    
    void init() override;
    void handle_events() override;

    lv_obj_t* get_screen() override {
        return screen;
    }
};