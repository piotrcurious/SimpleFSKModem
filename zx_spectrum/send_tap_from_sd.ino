#include <SD.h>
#include "TAPModem.h"

// Pins for SD and Audio
#define SD_CS_PIN 10
#define FSK_AUDIO_PIN 9

// Pins for Buttons
#define BTN_NEXT 7
#define BTN_PREV 6
#define BTN_SEND 3
#define BTN_EJECT 8 // New "Eject" button for select mode

TAPModem modem(FSK_AUDIO_PIN);
String currentTAPFile = "";
bool selectMode = true;
bool paused = false;

// Function prototypes
void selectNextTAPFile();
void selectPrevTAPFile();
void sendTAPFile(String fileName);

void setup() {
  Serial.begin(115200);
  modem.begin();

  if (!SD.begin(SD_CS_PIN)) {
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

void loop() {
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
  File root = SD.open("/");
  bool foundCurrent = (currentTAPFile == "");
  bool nextFound = false;

  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      if (nextFound) break;
      // Wrap around
      root.rewindDirectory();
      entry = root.openNextFile();
      if (!entry) break;
    }

    String name = entry.name();
    if (!entry.isDirectory() && name.endsWith(".tap")) {
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
  File root = SD.open("/");
  String lastTAP = "";
  String prevTAP = "";

  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;

    String name = entry.name();
    if (!entry.isDirectory() && name.endsWith(".tap")) {
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
  File tapFile = SD.open(fileName.c_str(), FILE_READ);
  if (!tapFile) {
    Serial.println("Failed to open file: " + fileName);
    return;
  }

  Serial.println("Sending: " + fileName);

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

    // Send first chunk from buffer
    for (int i = 0; i < readLen; i++) {
        if (bytesProcessed < length - 1) {
            modem.sendByte(chunkBuffer[i]);
            checksum ^= chunkBuffer[i];
            bytesProcessed++;
        }
    }

    // Send remaining chunks
    while (bytesProcessed < length - 1) {
        uint16_t toRead = (length - 1 - bytesProcessed > 512) ? 512 : (length - 1 - bytesProcessed);
        readLen = tapFile.read(chunkBuffer, (size_t)toRead);
        if (readLen <= 0) break;

        for (int i = 0; i < readLen; i++) {
            modem.sendByte(chunkBuffer[i]);
            checksum ^= chunkBuffer[i];
            bytesProcessed++;
        }
    }

    // Final byte (stored checksum)
    int checksumInt = tapFile.read();
    if (checksumInt >= 0) {
      byte storedChecksum = (byte)checksumInt;
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
