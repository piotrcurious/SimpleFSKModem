#include <SdFat.h>
#include "TAPModem.h"

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
#define BTN_EJECT 8 // New "Eject" button for select mode

// Pin for Turbo Knob
#define TURBO_KNOB_PIN A1

TAPModem modem(FSK_AUDIO_PIN);
String currentTAPFile = "";
bool selectMode = true;
bool paused = false;

// Function prototypes
void selectNextTAPFile();
void selectPrevTAPFile();
void sendTAPFile(String fileName);

float lastSpeedFactor = 1.0f;

void setup() {
  Serial.begin(115200);
  modem.begin();
  modem.beginInterrupt();

  if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
    Serial.println("SD initialization failed!");
    return;
  }

  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_SEND, INPUT_PULLUP);
  pinMode(BTN_EJECT, INPUT_PULLUP);

  Serial.println("TAP Sender Ready.");
  selectNextTAPFile();
}

void updateTurbo() {
  int val = analogRead(TURBO_KNOB_PIN);
  // Map 0-1023 to 1.0x - 3.0x speed
  float factor = 1.0f + ((float)val / 1023.0f) * 2.0f;

  // Only update if changed significantly (0.05 step) to avoid noise
  if (abs(factor - lastSpeedFactor) > 0.05f) {
    lastSpeedFactor = factor;
    modem.setSpeedFactor(factor);
    Serial.print("Speed: ");
    Serial.print((int)(factor * 100));
    Serial.println("%");
  }
}

void loop() {
  updateTurbo();
  if (digitalRead(BTN_EJECT) == LOW) {
    selectMode = true;
    paused = false;
    Serial.println("Eject: Entering Select Mode");
    delay(500);
  }

  if (selectMode) {
    if (digitalRead(BTN_NEXT) == LOW) {
      selectNextTAPFile();
      delay(500);
    }
    if (digitalRead(BTN_PREV) == LOW) {
      selectPrevTAPFile();
      delay(500);
    }
    if (digitalRead(BTN_SEND) == LOW) {
      selectMode = false;
      Serial.print("Ready to send: ");
      Serial.println(currentTAPFile);
      delay(500);
    }
  } else {
    if (digitalRead(BTN_SEND) == LOW) {
      sendTAPFile(currentTAPFile);
      delay(500);
    }
  }
}

void selectNextTAPFile() {
  SdFile root;
  if (!root.open("/")) {
      return;
  }
  bool foundCurrent = (currentTAPFile == "");
  bool nextFound = false;

  while (true) {
    SdFile entry;
    if (!entry.openNext(&root, O_RDONLY)) {
      if (nextFound) break;
      // Wrap around
      root.rewind();
      if (!entry.openNext(&root, O_RDONLY)) break;
    }

    char nameBuf[100];
    entry.getName(nameBuf, sizeof(nameBuf));
    String name = String(nameBuf);
    if (!entry.isDir() && name.endsWith(".tap")) {
      if (foundCurrent) {
        currentTAPFile = name;
        nextFound = true;
        break;
      }
      if (name == currentTAPFile) {
        foundCurrent = true;
      }
    }
    entry.close();
  }
  root.close();
  Serial.print("Selected TAP: ");
  Serial.println(currentTAPFile);
}

void selectPrevTAPFile() {
  SdFile root;
  if (!root.open("/")) {
      return;
  }
  String lastTAP = "";
  String prevTAP = "";

  while (true) {
    SdFile entry;
    if (!entry.openNext(&root, O_RDONLY)) break;

    char nameBuf[100];
    entry.getName(nameBuf, sizeof(nameBuf));
    String name = String(nameBuf);
    if (!entry.isDir() && name.endsWith(".tap")) {
      if (name == currentTAPFile) {
        if (lastTAP != "") prevTAP = lastTAP;
      }
      lastTAP = name;
    }
    entry.close();
  }

  if (prevTAP == "" && lastTAP != "") {
      prevTAP = lastTAP; // Wrap to last
  }

  if (prevTAP != "") {
    currentTAPFile = prevTAP;
  }

  root.close();
  Serial.print("Selected TAP: ");
  Serial.println(currentTAPFile);
}

void sendTAPFile(String fileName) {
  SdFile tapFile;
  if (!tapFile.open(fileName.c_str(), O_RDONLY)) {
    Serial.println("Failed to open file: " + fileName);
    return;
  }

  Serial.println("Sending: " + fileName);

  // 1-second leading silence
  modem.pause(1000);

  while (tapFile.available() >= 2) {
    // Check for EJECT or PAUSE
    if (digitalRead(BTN_EJECT) == LOW) {
      Serial.println("Interrupted by EJECT");
      selectMode = true;
      break;
    }

    // BTN_NEXT for Pause
    if (digitalRead(BTN_NEXT) == LOW) {
      paused = !paused;
      Serial.println(paused ? "Paused" : "Resumed");
      delay(500);
    }

    if (paused) {
      delay(100);
      continue;
    }

    // Read block length (16-bit little endian)
    int lsb = tapFile.read();
    if (lsb < 0) break;
    int msb = tapFile.read();
    if (msb < 0) break;
    uint16_t length = lsb | (msb << 8);

    if (length == 0) break;
    if (tapFile.available() < length) {
      Serial.println("Warning: Incomplete block");
      break;
    }

    // To ensure no gaps between Sync and Data, we read the first chunk of data
    // BEFORE starting the Pilot tone.
    // We use a 512-byte buffer (typical SD sector) to minimize latency.
    uint16_t bytesToRead = (length > 512) ? 512 : length;
    byte chunkBuffer[512];
    int readLen = tapFile.read(chunkBuffer, (size_t)bytesToRead);
    if (readLen <= 0) break;

    byte flagByte = chunkBuffer[0];

    Serial.print("Sending block, size: ");
    Serial.print(length);
    Serial.print(" Flag: 0x");
    Serial.println(flagByte, HEX);

    // Start playback: Pilot and Sync
    modem.sendPilot(flagByte == 0x00 ? TAP_PILOT_HEADER_PULSES : TAP_PILOT_DATA_PULSES);
    modem.sendSync();

    // Process the block
    byte checksum = 0;
    uint16_t bytesProcessed = 0;
    byte storedChecksum = 0;
    bool checksumRead = false;

    // Handle bytes from chunkBuffer
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

    // Send remaining chunks
    while (bytesProcessed < length) {
        updateTurbo(); // Allow updating speed mid-block
        uint16_t toRead = (length - bytesProcessed > 512) ? 512 : (length - bytesProcessed);
        readLen = tapFile.read(chunkBuffer, (size_t)toRead);
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

    // Final byte (checksum)
    if (checksumRead) {
      modem.sendByte(storedChecksum);

      if (checksum != storedChecksum) {
        Serial.print("Checksum Error! Calc: 0x");
        Serial.print(checksum, HEX);
        Serial.print(" File: 0x");
        Serial.println(storedChecksum, HEX);
      }
    }

    // Inter-block pause
    modem.pause(1000);
  }

  tapFile.close();
  if (!selectMode) {
    Serial.println("Done.");
  }
}
