#include <Arduino.h>

#include <lvgl.h>

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
SPIClass touchscreenSpi = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3700, touchScreenMinimumY = 240,touchScreenMaximumY = 3800;

#define TFT_HOR_RES   320
#define TFT_VER_RES   240

#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))

void flush_disp(lv_display_t *disp, const lv_area_t *area, uint8_t * px_map) {
  lv_disp_flush_ready(disp);
}

void my_touchpad_read( lv_indev_t * indev, lv_indev_data_t * data )
{
  if(touchscreen.touched())
  {
    TS_Point p = touchscreen.getPoint();
    //Some very basic auto calibration so it doesn't go out of range
    if(p.x < touchScreenMinimumX) touchScreenMinimumX = p.x;
    if(p.x > touchScreenMaximumX) touchScreenMaximumX = p.x;
    if(p.y < touchScreenMinimumY) touchScreenMinimumY = p.y;
    if(p.y > touchScreenMaximumY) touchScreenMaximumY = p.y;
    //Map this to the pixel position
    data->point.x = map(p.x,touchScreenMinimumX,touchScreenMaximumX,1,TFT_HOR_RES); /* Touchscreen X calibration */
    data->point.y = map(p.y,touchScreenMinimumY,touchScreenMaximumY,1,TFT_VER_RES); /* Touchscreen Y calibration */
    data->state = LV_INDEV_STATE_PRESSED;
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

lv_indev_t * indev;
uint8_t* draw_buf;
uint32_t lastTick = 0;

void listAllFiles() 
{
  File root = SD.open("/");
  File file = root.openNextFile();
  while(file){
    Serial.print("FILE: ");
    Serial.print(file.name());
    Serial.print("\tSIZE: ");
    Serial.println(file.size());
    file = root.openNextFile();
  }
}

void printSdInfo() {
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  listAllFiles();
}

bool isCardPresent() {
  File root = SD.open("/");
  if (!root) {
    return false;
  }
  root.close();
  return true;
}

void initSdCard() {
  // Initialize SD card with explicit VSPI pins
  SPIClass spi = SPIClass(VSPI);
  spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  
  if(!SD.begin(SD_CS, spi, 80000000)){
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
  printSdInfo();
}

void setup() {
  Serial.begin(115200);

  Serial.println("Initializing SD card...");
  initSdCard();

  //Initialise the touchscreen
  touchscreenSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); /* Start second SPI bus for touchscreen */
  touchscreen.begin(touchscreenSpi); /* Touchscreen init */
  touchscreen.setRotation(3); /* Inverted landscape orientation to match screen */

  //Initialise LVGL
  lv_init();
  draw_buf = new uint8_t[DRAW_BUF_SIZE];
  lv_display_t * disp;
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, DRAW_BUF_SIZE);

  //Initialize the XPT2046 input device driver
  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);  
  lv_indev_set_read_cb(indev, my_touchpad_read);
}

void loop() {
  lv_tick_inc(millis() - lastTick); //Update the tick timer. Tick is new for LVGL 9
  lastTick = millis();
  lv_timer_handler();               //Update the UI
  delay(5);
}