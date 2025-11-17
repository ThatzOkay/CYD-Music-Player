#pragma once
#include <lvgl.h>
#include "base_screen.h"

void on_songs_click();
void on_albums_click();
void on_shuffle_click();
void on_radio_click();
void on_settings_click();

class HomeScreen : public BaseScreen {
public:
    
    void init() override;
    void handle_events() override;

    lv_obj_t* get_screen() override {
        return screen;
    }
};