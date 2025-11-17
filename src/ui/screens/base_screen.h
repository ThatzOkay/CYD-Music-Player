#ifndef BASE_SCREEN_H
#define BASE_SCREEN_H

#include "lvgl.h"

class BaseScreen {
protected:
    lv_obj_t* screen = nullptr;

public:
    virtual ~BaseScreen() = default;

    virtual lv_obj_t* get_screen() {
        return screen;
    }
    
    virtual void init() = 0;
    virtual void handle_events() = 0;
};

#endif // BASE_SCREEN_H