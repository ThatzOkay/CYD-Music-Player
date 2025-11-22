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
<<<<<<< HEAD
#include "id3v1/parser.h"

#include <ESP32I2SAudio.h>
#include <BackgroundAudioAAC.h>
=======
>>>>>>> 24b08827dc0435804973717bc3cd4bb83402f255

SPIClass touchscreenSpi = SPIClass(VSPI);
SPIClass sdCardSpi = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3700, touchScreenMinimumY = 240, touchScreenMaximumY = 3800;

<<<<<<< HEAD
// Music list data shared with UI core
struct MusicEntry
{
  String path;
  String title;
  String artist;
  String album;
};

std::vector<MusicEntry> musicList;
SemaphoreHandle_t musicListMutex = nullptr;
bool musicListReady = false;

bool hasMp3Extension(const char *name)
{
  String s = String(name);
  s.toLowerCase();
  return s.endsWith(".mp3");
}

void scanMp3Directory(const char *dirname, std::vector<String> &files)
{
  File root = SD.open(dirname);
  if (!root || !root.isDirectory())
  {
    Serial.printf("Failed to open directory: %s\n", dirname);
    return;
  }

  File entry = root.openNextFile();
  while (entry)
  {
    if (entry.isDirectory())
    {
      String subDir = String(dirname) + "/" + entry.name();
      scanMp3Directory(subDir.c_str(), files);
    }
    else
    {
      if (hasMp3Extension(entry.name()))
      {
        String filePath = String(dirname) + "/" + entry.name();
        files.push_back(filePath);
        Serial.printf("Found: %s\n", filePath.c_str());
      }
    }
    entry.close();
    entry = root.openNextFile();
  }
  root.close();
}

=======
>>>>>>> 24b08827dc0435804973717bc3cd4bb83402f255
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
  sdCardSpi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, sdCardSpi, 80000000))
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

  Serial.println("SD card initialized successfully");
  // printSdInfo();
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

void appendCsvLine(const String &line)
{
  File f = SD.open("/music_index.csv", FILE_APPEND);
  if (!f)
  {
    Serial.println("Failed to open music_index.csv for writing!");
    return;
  }
  f.println(line);
  f.close();
}

String csvEscape(const String &s)
{
  String out = s;
  out.replace(",", " "); // simple escape to avoid breaking fields
  out.replace("\n", " ");
  return out;
}

std::vector<String> getAllMp3Files()
{
  std::vector<String> files;
  scanMp3Directory("/music", files);
  return files;
}

void initMusicList()
{
  Serial.println("Building music index...");

  auto files = getAllMp3Files();

  Serial.printf("Found %u MP3 files. Indexing...\n", files.size());

  // Create mutex for musicList access
  if (!musicListMutex)
  {
    musicListMutex = xSemaphoreCreateMutex();
  }

  for (auto &path : files)
  {
    File f = SD.open(path);
    if (!f)
    {
      Serial.printf("Failed to open %s\n", path.c_str());
      continue;
    }

    ID3v1Tag tag;
    bool hasTag = readID3v1(f, tag);

    if (!hasTag)
    {
      Serial.printf("No ID3v1 tag found in %s\n", path.c_str());
      tag.title[0] = '\0';
      tag.artist[0] = '\0';
      tag.album[0] = '\0';
    }

    f.close();

    MusicEntry entry;
    entry.path = path;
    entry.title = String(tag.title);
    entry.artist = String(tag.artist);
    entry.album = String(tag.album);

    if (musicListMutex)
      xSemaphoreTake(musicListMutex, portMAX_DELAY);
    musicList.push_back(entry);
    if (musicListMutex)
      xSemaphoreGive(musicListMutex);

    Serial.printf("Indexed: %s - %s\n", tag.artist, tag.title);
  }

  musicListReady = true;
  Serial.printf("Music list created with %u entries!\n", musicList.size());
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

<<<<<<< HEAD
void printMemoryUsage()
{
  Serial.println("===== Memory Usage =====");

  // General heap
  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("Largest free block: ");
  Serial.println(ESP.getMaxAllocHeap());
  Serial.print("Minimum free heap ever: ");
  Serial.println(ESP.getMinFreeHeap());
  // Serial.print("Heap fragmentation: ");
  // Serial.print(ESP.getHeapFragmentation());
  // Serial.println("%");

  // Internal DRAM (IRAM/DRAM segment)
  Serial.print("Internal DRAM free: ");
  Serial.println(esp_get_free_internal_heap_size());

  // LVGL heap usage (if using lv_mem_alloc)
  // Serial.print("LVGL heap free: ");
  // Serial.println(lv_mem_get_free_size());

  // Task stack usage (high water mark)
  Serial.print("LVGL task stack high-water mark: ");
  Serial.println(uxTaskGetStackHighWaterMark(NULL)); // Current task
  // You can add similar for audio task:
  // Serial.println(uxTaskGetStackHighWaterMark(audioTaskHandle));

  Serial.println("=======================");
}

=======
>>>>>>> 24b08827dc0435804973717bc3cd4bb83402f255
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

    static uint32_t lastMemPrint = 0;
    if (millis() - lastMemPrint > 5000)
    {
      // printMemoryUsage();
      lastMemPrint = millis();
    }

    delay(5);
  }
}

  ESP32I2SAudio audio(I2S_BCLK, I2S_LRCLK, I2S_DOUT);

void audioTask(void *pv)
{
  // Serial.println("Initializing SD card...");
  // initSdCard();

  // Serial.println("Audio task started on core 0!");
  // Serial.printf("Free heap: %d\\n", ESP.getFreeHeap());

  // // Wait for SD card to be initialized from setup()
  // vTaskDelay(pdMS_TO_TICKS(100));

  // // Build music index (all SD operations happen on this core)
  // Serial.println("Starting music indexing on audio core...");
  // initMusicList();

  // Serial.println("Initializing I2S audio...");
  // Serial.println("I2S initialized");

  // File f;
  // uint8_t filebuff[512];

  // Serial.println("Initializing AAC decoder...");
  // BackgroundAudioAAC BMP(audio);
  // Serial.println("AAC decoder initialized");

  // Serial.println("Opening audio file...");
  // // Open using SD library (no /sd prefix needed)
  // const char *filepath = "/music/Interlunium Feat. Kayli Mills - Electric Blue Supernova.mp3";
  // Serial.printf("Attempting to open: %s\\n", filepath);

  // f = SD.open(filepath, FILE_READ);

  // if (!f)
  // {
  //   Serial.printf("Failed to open audio file: %s\\n", filepath);
  //   Serial.println("Listing files in /music:");

  //   File root = SD.open("/music");
  //   if (root)
  //   {
  //     File entry = root.openNextFile();
  //     while (entry)
  //     {
  //       Serial.printf("  - %s\\n", entry.name());
  //       entry.close();
  //       entry = root.openNextFile();
  //     }
  //     root.close();
  //   }
  //   Serial.println("Failed to open audio file!");
  //   while (1)
  //   {
  //     vTaskDelay(pdMS_TO_TICKS(1));
  //   }
  // }

  // Serial.println("Audio file opened, starting playback...");

  // while (f)
  // {
  //   if (BMP.availableForWrite() > 512)
  //   {
  //     int len = f.read(filebuff, 512);
  //     if (len > 0)
  //     {
  //       BMP.write(filebuff, len);
  //     }

  //     if (len != 512)
  //     {
  //       Serial.println("End of file reached");
  //       f.close();
  //       break;
  //     }
  //   }

  //   // Yield to other tasks and feed watchdog
  //   vTaskDelay(pdMS_TO_TICKS(1));
  // }

  // Serial.println("Playback finished");

  // Keep task alive
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(LDR_PIN, INPUT);

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

  // Music list will be initialized in audioTask (core 0)

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
      16384,
      NULL,
      1,
      NULL,
      0);
}

void loop()
{
  vTaskDelay(portMAX_DELAY);
}