#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

struct WiFiConfig {
    bool enable;
    char ssid[32];
    char password[64];
    bool autoConnect;
    int connectionTimeout;
};

struct AudioConfig {
    int volume;
    bool muteOnStart;
};

struct DisplayConfig {
    int brightness;
    bool autoBrightness;
};

struct CalibrationConfig {
    int touchMinX;
    int touchMaxX;
    int touchMinY;
    int touchMaxY;
};

struct Config {
    WiFiConfig wifi;
    AudioConfig audio;
    DisplayConfig display;
    CalibrationConfig calibration;
};

// External declaration - the actual instance will be defined in config.cpp
extern Config config;

// Function declarations
bool loadConfig();
bool saveConfig();
void setDefaultConfig();

#endif // CONFIG_H