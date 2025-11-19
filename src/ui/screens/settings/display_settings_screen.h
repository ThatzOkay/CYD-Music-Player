#include "../base_screen.h"

class DisplaySettingsScreen : public BaseScreen {
public:
    void init() override;
    void handle_events() override;

    lv_obj_t* get_screen() override {
        return screen;
    }
};