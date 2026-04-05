#include <SdFat.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include "TAPModem.h"

// OLED Display settings (using SSD1306Ascii for zero-RAM buffer)
#define I2C_ADDRESS 0x3C
SSD1306AsciiWire display;

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

const char* statusMsg = "Ready";
float lastSpeedFactor = 1.0f;

void updateDisplay();
void refreshFileList();
void sendTAPFile(String fileName);

void setup() {
  Serial.begin(115200);
  modem.begin();
  modem.beginInterrupt();

  Wire.begin();
  Wire.setClock(400000L);
  display.begin(&Adafruit128x64, I2C_ADDRESS);
  display.setFont(Adafruit5x7);
  display.clear();
  display.println(F("Initializing..."));

  if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
    display.clear();
    display.println(F("SD init failed!"));
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
        if (selectedIndex >= topIndex + 6) topIndex++;
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
  display.clear();

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
        if (count >= topIndex && line < 6) {
          if (count == selectedIndex) {
            display.setInvertMode(true);
            display.print(F(">"));
          } else {
            display.setInvertMode(false);
            display.print(F(" "));
          }
          display.println(name.substring(0, 20));
          display.setInvertMode(false);
          line++;
        }
        count++;
      }
      entry.close();
    }
    root.close();

    display.setCursor(0, 7); // Row 7 (8th line)
    display.print(selectedIndex + 1);
    display.print(F("/"));
    display.print(totalFiles);
  } else {
    display.setCursor(0, 0);
    display.println(F("--- Playback ---"));
    display.println(currentTAPFile);

    display.print(F("Status: "));
    display.println(statusMsg);

    display.print(F("Speed: "));
    display.print((int)(lastSpeedFactor * 100));
    display.println(F("%"));

    if (paused) {
      display.setCursor(0, 6);
      display.println(F("[PAUSE]"));
    }
  }
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

    // Quick status update on line 5
    display.setCursor(0, 5);
    display.print(F("Blk: "));
    display.print(length);
    display.print(F(" F:"));
    display.print(flagByte, HEX);
    display.print(F("      ")); // Clear rest of line

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
