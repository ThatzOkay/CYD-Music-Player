#include "config.h"
#include <Preferences.h>

// Global config instance
Config config;

static Preferences preferences;

// Initialize storage
bool initStorage() {
    bool success = preferences.begin("config", false);
    if (!success) {
        Serial.println("Failed to initialize Preferences");
    }
    return success;
}

// Default configuration values
void setDefaultConfig() {
    // WiFi defaults
    config.wifi.enable = false;
    strcpy(config.wifi.ssid, "");
    strcpy(config.wifi.password, "");
    config.wifi.autoConnect = false;
    config.wifi.connectionTimeout = 10000;
    
    // Audio defaults
    config.audio.volume = 50;
    config.audio.muteOnStart = false;
    
    // Display defaults
    config.display.brightness = 128;
    config.display.autoBrightness = true;

    // Calibration defaults
    config.calibration.touchMinX = 0;
    config.calibration.touchMaxX = 4095;
    config.calibration.touchMinY = 0;
    config.calibration.touchMaxY = 4095;
}

bool loadConfig() {
    if (!initStorage()) {
        Serial.println("No storage available, using defaults");
        setDefaultConfig();
        return false;
    }
    
    config.wifi.enable = preferences.getBool("wifi_enable", false);
    preferences.getString("wifi_ssid", config.wifi.ssid, sizeof(config.wifi.ssid));
    preferences.getString("wifi_pass", config.wifi.password, sizeof(config.wifi.password));
    config.wifi.autoConnect = preferences.getBool("wifi_auto", false);
    config.wifi.connectionTimeout = preferences.getUInt("wifi_timeout", 10000);
    
    config.audio.volume = preferences.getUInt("audio_vol", 50);
    config.audio.muteOnStart = preferences.getBool("audio_mute", false);
    
    config.display.brightness = preferences.getUInt("disp_bright", 128);
    config.display.autoBrightness = preferences.getBool("disp_auto", true);
    
    config.calibration.touchMinX = preferences.getInt("calib_min_x", 0);
    config.calibration.touchMaxX = preferences.getInt("calib_max_x", 4095);
    config.calibration.touchMinY = preferences.getInt("calib_min_y", 0);
    config.calibration.touchMaxY = preferences.getInt("calib_max_y", 4095);

    preferences.end();
    return true;
}

// Save configuration to storage
bool saveConfig() {
    if (!initStorage()) {
        Serial.println("No storage available");
        return false;
    }
    
    preferences.putBool("wifi_enable", config.wifi.enable);
    preferences.putString("wifi_ssid", config.wifi.ssid);
    preferences.putString("wifi_pass", config.wifi.password);
    preferences.putBool("wifi_auto", config.wifi.autoConnect);
    preferences.putUInt("wifi_timeout", config.wifi.connectionTimeout);
    
    preferences.putUInt("audio_vol", config.audio.volume);
    preferences.putBool("audio_mute", config.audio.muteOnStart);
    
    preferences.putUInt("disp_bright", config.display.brightness);
    preferences.putBool("disp_auto", config.display.autoBrightness);
    
    preferences.putInt("calib_min_x", config.calibration.touchMinX);
    preferences.putInt("calib_max_x", config.calibration.touchMaxX);
    preferences.putInt("calib_min_y", config.calibration.touchMinY);
    preferences.putInt("calib_max_y", config.calibration.touchMaxY);

    preferences.end();
    Serial.println("Config saved successfully to Preferences");
    return true;
}
