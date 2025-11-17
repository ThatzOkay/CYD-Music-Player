#include <Arduino.h>

#include <lvgl.h>

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "ui/ui.h"

#include "config.h"
#include <WiFi.h>

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
SPIClass touchscreenSpi = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3700, touchScreenMinimumY = 240, touchScreenMaximumY = 3800;

#define LDR_PIN 34
#define BACKLIGHT_PIN 21

#define TFT_HOR_RES 320
#define TFT_VER_RES 240

#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))

void flush_disp(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  if (touchscreen.touched())
  {
    TS_Point p = touchscreen.getPoint();
    // Some very basic auto calibration so it doesn't go out of range
    if (p.x < touchScreenMinimumX)
      touchScreenMinimumX = p.x;
    if (p.x > touchScreenMaximumX)
      touchScreenMaximumX = p.x;
    if (p.y < touchScreenMinimumY)
      touchScreenMinimumY = p.y;
    if (p.y > touchScreenMaximumY)
      touchScreenMaximumY = p.y;
    // Map this to the pixel position
    data->point.x = map(p.x, touchScreenMinimumX, touchScreenMaximumX, 1, TFT_HOR_RES); /* Touchscreen X calibration */
    data->point.y = map(p.y, touchScreenMinimumY, touchScreenMaximumY, 1, TFT_VER_RES); /* Touchscreen Y calibration */
    data->state = LV_INDEV_STATE_PRESSED;
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

lv_indev_t *indev;
uint8_t *draw_buf;
uint32_t lastTick = 0;
uint32_t lastBrightnessUpdate = 0;

void listAllFiles()
{
  File root = SD.open("/music");
  File file = root.openNextFile();
  while (file)
  {
    Serial.print("FILE: ");
    Serial.print(file.name());
    Serial.print("\tSIZE: ");
    Serial.println(file.size());
    file = root.openNextFile();
  }
}

void printSdInfo()
{
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  listAllFiles();
}

void initSdCard()
{
  // Initialize SD card with explicit VSPI pins
  SPIClass spi = SPIClass(VSPI);
  spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, spi, 80000000))
  {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }

  printSdInfo();
}

void initTouchScreen()
{
  touchscreenSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); /* Start second SPI bus for touchscreen */
  touchscreen.begin(touchscreenSpi);                                         /* Touchscreen init */
  touchscreen.setRotation(3);
}

void initLvglDisplay()
{
  lv_init();
  draw_buf = new uint8_t[DRAW_BUF_SIZE];
  lv_display_t *disp;
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, DRAW_BUF_SIZE);
}

void initLvglTouch()
{
  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);
}

void initBrightness()
{
  pinMode(LDR_PIN, INPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  ledcSetup(0, 5000, 8); // Channel 0, 5kHz, 8-bit resolution
  ledcAttachPin(BACKLIGHT_PIN, 0);
}

void updateBrightness()
{
  int ldrValue = analogRead(LDR_PIN);
  // Map LDR reading (0-4095) to brightness (50-255)
  // Lower LDR value = darker environment = lower brightness
  int brightness = map(ldrValue, 0, 300, 50, 255);
  brightness = constrain(brightness, 50, 255);
  ledcWrite(0, brightness);
}

void setup()
{
  Serial.begin(115200);

  analogSetAttenuation(ADC_0db);

  pinMode(LDR_PIN, INPUT);

  Serial.println("Initializing SD card...");
  initSdCard();

  loadConfig();

  if (strlen(config.wifi.ssid) > 0 && strlen(config.wifi.password) > 0) {
    Serial.println("WiFi credentials found, connecting...");
    WiFi.begin(config.wifi.ssid, config.wifi.password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.print("WiFi connected! IP address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println();
      Serial.println("WiFi connection failed");
    }
  } else {
    Serial.println("No WiFi credentials configured");
  }

  initTouchScreen();

  initLvglDisplay();

  initLvglTouch();

  initBrightness();

  music_player_ui_init();
}

void loop()
{
  lv_tick_inc(millis() - lastTick);
  lastTick = millis();
  lv_timer_handler();
  
  if (millis() - lastBrightnessUpdate > 1000)
  {
    updateBrightness();
    lastBrightnessUpdate = millis();
  }

  music_player_ui_handle_events();
  
  delay(5);
}