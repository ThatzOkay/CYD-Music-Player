#include <Arduino.h>

#include <lvgl.h>

#include <TFT_eSPI.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "ui/ui.h"

#include "config.h"
#include <WiFi.h>
#include "commons.h"
#include <XPT2046_Touchscreen.h>

SPIClass touchscreenSpi = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3700, touchScreenMinimumY = 240, touchScreenMaximumY = 3800;

void flush_disp(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  if (touchscreen.touched())
  {
    TS_Point p = touchscreen.getPoint();

    data->point.x = map(p.x,
                        config.calibration.touchMinX,
                        config.calibration.touchMaxX,
                        0, TFT_HOR_RES - 1);

    data->point.y = map(p.y,
                        config.calibration.touchMinY,
                        config.calibration.touchMaxY,
                        0, TFT_VER_RES - 1);

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

  // Setting up the LEDC and configuring the Back light pin
  // NOTE: this needs to be done after tft.init()
#if ESP_IDF_VERSION_MAJOR == 5
  ledcAttach(BACKLIGHT_PIN, 5000, LEDC_TIMER_12_BIT);
#else
  ledcSetup(LEDC_CHANNEL_0, 5000, LEDC_TIMER_12_BIT);
  ledcAttachPin(BACKLIGHT_PIN, LEDC_CHANNEL_0);
#endif
}

void updateBrightness()
{
  if (config.display.autoBrightness == false)
  {
    ledcWriteChannel(LEDC_CHANNEL_0, config.display.brightness);
  }
  else
  {
    int ldrValue = analogRead(LDR_PIN);
    // Map LDR reading (0-4095) to brightness (50-255)
    // Lower LDR value = darker environment = lower brightness
    int brightness = map(ldrValue, 0, 300, 50, 255);
    brightness = constrain(brightness, 50, 255);
    ledcWriteChannel(LEDC_CHANNEL_0, brightness);
  }
}

void lvglTask(void *pv)
{
  while (1)
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
}

void audioTask(void *pv)
{
  while (1)
  {
    // Placeholder for audio handling code
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(LDR_PIN, INPUT);

  Serial.println("Initializing SD card...");
  initSdCard();

  loadConfig();

  pinMode(0, INPUT_PULLUP);
  if (digitalRead(0) == LOW)
  {
    Serial.println("Boot button held - resetting config...");
    setDefaultConfig();
    saveConfig();
    Serial.println("Config reset complete!");
  }

  if (strlen(config.wifi.ssid) > 0 && strlen(config.wifi.password) > 0)
  {
    Serial.println("WiFi credentials found, connecting...");
    WiFi.begin(config.wifi.ssid, config.wifi.password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println();
      Serial.print("WiFi connected! IP address: ");
      Serial.println(WiFi.localIP());
    }
    else
    {
      Serial.println();
      Serial.println("WiFi connection failed");
    }
  }
  else
  {
    Serial.println("No WiFi credentials configured");
  }

  initTouchScreen();

  initLvglDisplay();

  initLvglTouch();

  initBrightness();

  music_player_ui_init();

  xTaskCreatePinnedToCore(
      lvglTask,
      "LVGL",
      7168,
      NULL,
      1,
      NULL,
      1);

  xTaskCreatePinnedToCore(
      audioTask,
      "Audio",
      2048,
      NULL,
      1,
      NULL,
      0);
}

void loop()
{
  vTaskDelay(portMAX_DELAY);
}