#include <SdFat.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "TAPModem.h"

// OLED Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pins for SD and Audio
#define SD_CS_PIN 10
#ifndef sd
SdFat sd;
#endif
#define FSK_AUDIO_PIN 9

// Pins for Buttons
#define BTN_NEXT 7
#define BTN_PREV 6
#define BTN_SEND 3
#define BTN_EJECT 8

// Pin for Turbo Knob
#define TURBO_KNOB_PIN A1

TAPModem modem(FSK_AUDIO_PIN);
int selectedIndex = 0;
int topIndex = 0;
int totalFiles = 0;
String currentTAPFile = "";
bool selectMode = true;
bool paused = false;

String statusMsg = "Ready";
float lastSpeedFactor = 1.0f;

void updateDisplay();
void refreshFileList();
void sendTAPFile(String fileName);

void setup() {
  Serial.begin(115200);
  modem.begin();
  modem.beginInterrupt();

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(F("Initializing..."));
  display.display();

  if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println(F("SD init failed!"));
    display.display();
    return;
  }

  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_SEND, INPUT_PULLUP);
  pinMode(BTN_EJECT, INPUT_PULLUP);

  refreshFileList();
  updateDisplay();
}

void updateTurbo() {
  int val = analogRead(TURBO_KNOB_PIN);
  float factor = 1.0f + ((float)val / 1023.0f) * 2.0f;
  if (abs(factor - lastSpeedFactor) > 0.05f) {
    lastSpeedFactor = factor;
    modem.setSpeedFactor(factor);
    if (!selectMode) updateDisplay();
  }
}

void loop() {
  updateTurbo();

  if (digitalRead(BTN_EJECT) == LOW) {
    selectMode = true;
    paused = false;
    statusMsg = "Select File";
    updateDisplay();
    delay(500);
  }

  if (selectMode) {
    if (digitalRead(BTN_NEXT) == LOW) {
      if (selectedIndex < totalFiles - 1) {
        selectedIndex++;
        if (selectedIndex >= topIndex + 5) topIndex++;
        updateDisplay();
      }
      delay(200);
    }
    if (digitalRead(BTN_PREV) == LOW) {
      if (selectedIndex > 0) {
        selectedIndex--;
        if (selectedIndex < topIndex) topIndex--;
        updateDisplay();
      }
      delay(200);
    }
    if (digitalRead(BTN_SEND) == LOW) {
      SdFile root;
      root.open("/");
      int count = 0;
      SdFile entry;
      while (entry.openNext(&root, O_RDONLY)) {
        char nameBuf[100];
        entry.getName(nameBuf, sizeof(nameBuf));
        String name = String(nameBuf);
        if (!entry.isDir() && name.endsWith(".tap")) {
          if (count == selectedIndex) {
            currentTAPFile = name;
            entry.close();
            break;
          }
          count++;
        }
        entry.close();
      }
      root.close();

      selectMode = false;
      statusMsg = "Ready";
      updateDisplay();
      delay(500);
    }
  } else {
    if (digitalRead(BTN_SEND) == LOW) {
      statusMsg = "Sending...";
      updateDisplay();
      sendTAPFile(currentTAPFile);
      statusMsg = "Done";
      updateDisplay();
      delay(500);
    }
  }
}

void refreshFileList() {
  totalFiles = 0;
  SdFile root;
  if (!root.open("/")) return;
  SdFile entry;
  while (entry.openNext(&root, O_RDONLY)) {
    char nameBuf[100];
    entry.getName(nameBuf, sizeof(nameBuf));
    String name = String(nameBuf);
    if (!entry.isDir() && name.endsWith(".tap")) {
      totalFiles++;
    }
    entry.close();
  }
  root.close();
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);

  if (selectMode) {
    display.setCursor(0, 0);
    display.println(F("--- Select TAP ---"));

    SdFile root;
    root.open("/");
    SdFile entry;
    int count = 0;
    int line = 0;
    while (entry.openNext(&root, O_RDONLY)) {
      char nameBuf[100];
      entry.getName(nameBuf, sizeof(nameBuf));
      String name = String(nameBuf);
      if (!entry.isDir() && name.endsWith(".tap")) {
        if (count >= topIndex && line < 5) {
          display.setCursor(0, 12 + line * 10);
          if (count == selectedIndex) {
            display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Invert for selection
            display.print(F(">"));
          } else {
            display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
            display.print(F(" "));
          }
          display.println(name.substring(0, 20));
          line++;
        }
        count++;
      }
      entry.close();
    }
    root.close();
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // Reset

    display.setCursor(0, 56);
    display.print(selectedIndex + 1);
    display.print(F("/"));
    display.print(totalFiles);
  } else {
    display.setCursor(0, 0);
    display.println(F("--- Playback ---"));
    display.setCursor(0, 15);
    display.println(currentTAPFile);

    display.setCursor(0, 30);
    display.print(F("Status: "));
    display.println(statusMsg);

    display.setCursor(0, 45);
    display.print(F("Speed: "));
    display.print((int)(lastSpeedFactor * 100));
    display.println(F("%"));

    if (paused) {
      display.setCursor(90, 30);
      display.println(F("[PAUSE]"));
    }
  }

  display.display();
}

void sendTAPFile(String fileName) {
  SdFile tapFile;
  if (!tapFile.open(fileName.c_str(), O_RDONLY)) return;

  modem.pause(1000);

  while (tapFile.available() >= 2) {
    if (digitalRead(BTN_EJECT) == LOW) {
      selectMode = true;
      break;
    }

    if (digitalRead(BTN_NEXT) == LOW) {
      paused = !paused;
      updateDisplay();
      delay(500);
    }

    if (paused) {
      delay(100);
      continue;
    }

    int lsb = tapFile.read();
    int msb = tapFile.read();
    uint16_t length = lsb | (msb << 8);
    if (length == 0) break;

    uint16_t bytesToRead = (length > 512) ? 512 : length;
    byte chunkBuffer[512];
    int readLen = (int)tapFile.read(chunkBuffer, (size_t)bytesToRead);
    if (readLen <= 0) break;

    byte flagByte = chunkBuffer[0];

    display.fillRect(0, 30, 128, 10, SSD1306_BLACK);
    display.setCursor(0, 30);
    display.print(F("Block: "));
    display.print(length);
    display.print(F(" F:"));
    display.print(flagByte, HEX);
    display.display();

    modem.sendPilot(flagByte == 0x00 ? TAP_PILOT_HEADER_PULSES : TAP_PILOT_DATA_PULSES);
    modem.sendSync();

    uint16_t bytesProcessed = 0;
    byte storedChecksum = 0;
    bool checksumRead = false;
    byte checksum = 0;

    for (int i = 0; i < readLen; i++) {
        if (bytesProcessed < length - 1) {
            modem.sendByte(chunkBuffer[i]);
            checksum ^= chunkBuffer[i];
            bytesProcessed++;
        } else if (bytesProcessed == length - 1) {
            storedChecksum = chunkBuffer[i];
            checksumRead = true;
            bytesProcessed++;
        }
    }

    while (bytesProcessed < length) {
        uint16_t toRead = (length - bytesProcessed > 512) ? 512 : (length - bytesProcessed);
        readLen = (int)tapFile.read(chunkBuffer, (size_t)toRead);
        if (readLen <= 0) break;
        for (int i = 0; i < readLen; i++) {
            if (bytesProcessed < length - 1) {
                modem.sendByte(chunkBuffer[i]);
                checksum ^= chunkBuffer[i];
                bytesProcessed++;
            } else if (bytesProcessed == length - 1) {
                storedChecksum = chunkBuffer[i];
                checksumRead = true;
                bytesProcessed++;
            }
        }
    }

    if (checksumRead) modem.sendByte(storedChecksum);
    modem.pause(1000);
  }
  tapFile.close();
}
