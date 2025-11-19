#include "wifi_settings_screen.h"
#include <ui/ui.h>
#include <ui/components/settings/settings_button.h>
#include <ui/components/settings/settings_card.h>
#include "settings_screen.h"
#include "config.h"
#include <WiFi.h>
#include <vector>
#include <algorithm>

extern Config config;
extern bool saveConfig();

lv_obj_t *wifi_networks_card = nullptr;
bool currently_scanning = false;
std::vector<String> found_networks;

void refresh_wifi_list();

void connect_to_wifi(const char *ssid, const char *password)
{
    Serial.printf("Connecting to %s...\n", ssid);

    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nConnected!");

        // Save to config
        strcpy(config.wifi.ssid, ssid);
        strcpy(config.wifi.password, password);
        config.wifi.enable = true;
        config.wifi.autoConnect = true;
        saveConfig();

        refresh_wifi_list();
    }
    else
    {
        Serial.println("\nConnection failed!");
    }
}

void show_password_dialog(const char *ssid)
{
    lv_obj_t *overlay = lv_obj_create(lv_scr_act());
    lv_obj_set_size(overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_50, 0);
    lv_obj_set_style_border_width(overlay, 0, 0);
    lv_obj_set_style_radius(overlay, 0, 0);
    lv_obj_set_style_pad_all(overlay, 0, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_flag(overlay, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_FLOATING);

    lv_obj_t *dialog = lv_obj_create(overlay);
    lv_obj_set_size(dialog, lv_pct(90), LV_SIZE_CONTENT);
    lv_obj_align(dialog, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_bg_color(dialog, lv_color_white(), 0);
    lv_obj_set_style_radius(dialog, 12, 0);
    lv_obj_set_style_pad_all(dialog, 20, 0);
    lv_obj_set_style_shadow_width(dialog, 20, 0);
    lv_obj_set_style_shadow_color(dialog, lv_color_black(), 0);
    lv_obj_set_style_shadow_opa(dialog, LV_OPA_30, 0);

    lv_obj_set_flex_flow(dialog, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(dialog, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(dialog, 10, 0);

    lv_obj_t *title = lv_label_create(dialog);
    lv_label_set_text_fmt(title, "Connect to %s", ssid);
    lv_obj_set_style_text_color(title, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);

    lv_obj_t *password_input = lv_textarea_create(dialog);
    lv_obj_set_width(password_input, lv_pct(100));
    lv_textarea_set_placeholder_text(password_input, "Password");
    lv_textarea_set_password_mode(password_input, true);
    lv_textarea_set_one_line(password_input, true);

    lv_obj_add_event_cb(password_input, [](lv_event_t *e)
                        {
        lv_obj_t *password_input = (lv_obj_t*)lv_event_get_target(e);
        lv_obj_t *dialog = (lv_obj_t*)lv_obj_get_parent(password_input);
        lv_obj_t *overlay = (lv_obj_t*)lv_obj_get_parent(dialog);
        lv_obj_t *keyboard = lv_keyboard_create(overlay);
        lv_obj_set_size(keyboard, lv_pct(100), lv_pct(40));
        lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_keyboard_set_textarea(keyboard, password_input);

        lv_obj_set_user_data(overlay, keyboard); 
    }, LV_EVENT_FOCUSED, NULL);

    lv_obj_add_event_cb(password_input, [](lv_event_t *e)
                        {
        lv_obj_t *password_input = (lv_obj_t*)lv_event_get_target(e);
        lv_obj_t *dialog = (lv_obj_t*)lv_obj_get_parent(password_input);
        lv_obj_t *overlay = (lv_obj_t*)lv_obj_get_parent(dialog);
        lv_obj_t *keyboard = (lv_obj_t*)lv_obj_get_user_data(overlay);
        if (keyboard != NULL) {
            lv_obj_del(keyboard);
            lv_obj_set_user_data(overlay, NULL); 
        }
    }, LV_EVENT_DEFOCUSED, NULL);

    lv_obj_t *btn_container = lv_obj_create(dialog);
    lv_obj_remove_style_all(btn_container);
    lv_obj_set_width(btn_container, lv_pct(100));
    lv_obj_set_height(btn_container, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *cancel_btn = lv_btn_create(btn_container);
    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "Cancel");
    lv_obj_add_event_cb(cancel_btn, [](lv_event_t *e)
                        {
        lv_obj_t *dialog = (lv_obj_t*)lv_obj_get_parent(lv_obj_get_parent((lv_obj_t*)lv_event_get_target(e)));
        lv_obj_t *overlay = (lv_obj_t*)lv_obj_get_parent(dialog);
        lv_obj_del(overlay); }, LV_EVENT_CLICKED, NULL);

    lv_obj_t *connect_btn = lv_btn_create(btn_container);
    lv_obj_t *connect_label = lv_label_create(connect_btn);
    lv_label_set_text(connect_label, "Connect");

    char *ssid_copy = (char *)malloc(strlen(ssid) + 1);
    strcpy(ssid_copy, ssid);

    lv_obj_add_event_cb(connect_btn, [](lv_event_t *e)
                        {
        lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
        lv_obj_t *dialog = (lv_obj_t*)lv_obj_get_parent(lv_obj_get_parent(btn));
        lv_obj_t *password_input = (lv_obj_t*)lv_obj_get_child(dialog, 1);
        const char *password = lv_textarea_get_text(password_input);
        const char *ssid = (const char*)lv_event_get_user_data(e);
        
        connect_to_wifi(ssid, password);
        
        free((void*)ssid);
        lv_obj_t *overlay = (lv_obj_t*)lv_obj_get_parent(dialog);
        lv_obj_del(overlay); }, LV_EVENT_CLICKED, ssid_copy);
}

void init_wifi_networks_card(lv_obj_t *parent)
{
    wifi_networks_card = ui_settings_card(parent);
    found_networks.clear();

    lv_obj_t *networks_title = lv_label_create(wifi_networks_card);
    lv_label_set_text(networks_title, "Scanning...");
    lv_obj_set_style_text_color(networks_title, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(networks_title, &lv_font_montserrat_14, 0);

    Serial.println("Starting WiFi scan...");
    WiFi.scanNetworks(true);
    currently_scanning = true;
}

void update_wifi_networks()
{
    if (wifi_networks_card == nullptr || !currently_scanning)
        return;

    int16_t scanResult = WiFi.scanComplete();

    if (scanResult == WIFI_SCAN_RUNNING)
    {
        return;
    }

    if (scanResult >= 0)
    {
        currently_scanning = false;

        found_networks.clear();
        for (int i = 0; i < scanResult; i++)
        {
            String ssid = WiFi.SSID(i);
            bool isDuplicate = false;
            for (const String &existing : found_networks)
            {
                if (existing == ssid)
                {
                    isDuplicate = true;
                    break;
                }
            }
            if (!isDuplicate && ssid.length() > 0)
            {
                found_networks.push_back(ssid);
            }
        }

        lv_obj_clean(wifi_networks_card);

        if (found_networks.size() == 0)
        {
            lv_obj_t *no_networks_label = lv_label_create(wifi_networks_card);
            lv_label_set_text(no_networks_label, "No networks found");
            lv_obj_set_style_text_color(no_networks_label, lv_color_hex(0x000000), 0);
        }
        else
        {
            lv_obj_t *networks_title = lv_label_create(wifi_networks_card);
            char title[32];
            snprintf(title, sizeof(title), "Found %d Networks", found_networks.size());
            lv_label_set_text(networks_title, title);
            lv_obj_set_style_text_color(networks_title, lv_color_hex(0x000000), 0);
            lv_obj_set_style_text_font(networks_title, &lv_font_montserrat_14, 0);

            String currentSSID = WiFi.SSID();
            bool isConnected = WiFi.status() == WL_CONNECTED;

            for (size_t i = 0; i < found_networks.size() && i < 10; i++)
            {
                lv_obj_t *network_container = lv_obj_create(wifi_networks_card);
                lv_obj_remove_style_all(network_container);
                lv_obj_set_width(network_container, lv_pct(100));
                lv_obj_set_height(network_container, LV_SIZE_CONTENT);
                lv_obj_set_flex_flow(network_container, LV_FLEX_FLOW_COLUMN);
                lv_obj_set_flex_align(network_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
                lv_obj_set_style_pad_row(network_container, 2, 0);

                char *ssid_copy = (char *)malloc(found_networks[i].length() + 1);
                strcpy(ssid_copy, found_networks[i].c_str());

                lv_obj_t *btn = ui_settings_button(network_container, LV_SYMBOL_WIFI, found_networks[i].c_str(), nullptr);

                lv_obj_add_event_cb(btn, [](lv_event_t *e)
                                    {
                    char* ssid = (char*)lv_event_get_user_data(e);
                    String currentSSID = WiFi.SSID();
                    bool isConnected = WiFi.status() == WL_CONNECTED;
                    if (isConnected && String(ssid) == currentSSID) {
                        return;
                    }
                    show_password_dialog(ssid); }, LV_EVENT_CLICKED, ssid_copy);

                if (isConnected && found_networks[i] == currentSSID)
                {
                    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
                    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2493F0), 0);
                }
            }
        }

        WiFi.scanDelete();
    } else {
        Serial.println("WiFi scan failed");
        currently_scanning = false;
    }
}

void refresh_wifi_list()
{
    if (wifi_networks_card != nullptr)
    {
        currently_scanning = true;

        lv_obj_clean(wifi_networks_card);

        lv_obj_t *networks_title = lv_label_create(wifi_networks_card);
        lv_label_set_text(networks_title, "Refreshing...");
        lv_obj_set_style_text_color(networks_title, lv_color_hex(0x000000), 0);
        lv_obj_set_style_text_font(networks_title, &lv_font_montserrat_14, 0);

        WiFi.scanNetworks(true);
    }
}

void WifiSettingsScreen::init()
{
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101018), 0);

    lv_obj_set_style_pad_bottom(screen, 0, 0);

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
    lv_label_set_text(title, "WiFi Settings");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);

    lv_obj_t *top_card = ui_settings_card(wrapper);

    ui_settings_button(top_card, LV_SYMBOL_LEFT, "Back", []()
                       { switch_screen(new SettingsScreen()); });

    lv_obj_t *wifi_card = ui_settings_card(wrapper);

    lv_obj_t *wifi_enable_wrappper = lv_obj_create(wifi_card);
    lv_obj_remove_style_all(wifi_enable_wrappper);

    lv_obj_set_width(wifi_enable_wrappper, lv_pct(100));
    lv_obj_set_height(wifi_enable_wrappper, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(wifi_enable_wrappper, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(wifi_enable_wrappper,
                          LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_pad_column(wifi_enable_wrappper, 8, 0);

    lv_obj_t *wifi_enable_label = lv_label_create(wifi_enable_wrappper);
    lv_label_set_text(wifi_enable_label, "WiFi");
    lv_obj_set_style_text_color(wifi_enable_label, lv_color_hex(0x000000), 0);

    lv_obj_t *wifi_enable_switch = lv_switch_create(wifi_enable_wrappper);

    if (config.wifi.enable)
    {
        lv_obj_add_state(wifi_enable_switch, LV_STATE_CHECKED);
        init_wifi_networks_card(wrapper);
    }

    lv_obj_add_event_cb(wifi_enable_switch, [](lv_event_t *e)
                        {
        lv_obj_t *sw = (lv_obj_t *)lv_event_get_target(e);
        if(lv_obj_has_state(sw, LV_STATE_CHECKED)) {
            config.wifi.enable = true;
            saveConfig();
            WiFi.mode(WIFI_STA);
            Serial.println("WiFi enabled");
            
            if (wifi_networks_card == nullptr) {
                lv_obj_t *wrapper = lv_obj_get_parent(lv_obj_get_parent(lv_obj_get_parent(sw)));
                init_wifi_networks_card(wrapper);
            }
            
            // Try non-blocking auto-connect after starting scan
            if (strlen(config.wifi.ssid) > 0 && config.wifi.autoConnect) {
                Serial.printf("Auto-connecting to %s...\n", config.wifi.ssid);
                WiFi.begin(config.wifi.ssid, config.wifi.password);
            }
        } else {
            config.wifi.enable = false;
            saveConfig();
            
            WiFi.disconnect(true);
            WiFi.mode(WIFI_OFF);
            Serial.println("WiFi disabled and disconnected");
            
            if (wifi_networks_card != nullptr) {
                lv_obj_del(wifi_networks_card);
                wifi_networks_card = nullptr;
            }
        } }, LV_EVENT_VALUE_CHANGED, NULL);
}

void WifiSettingsScreen::handle_events()
{
    update_wifi_networks();
}