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

    // BTN_NEXT could act as pause/resume in play mode if we were mid-file
    // but sendTAPFile is currently blocking. To support pause, we need to
    // refactor sendTAPFile to be non-blocking.
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
    int msb = tapFile.read();
    uint16_t length = lsb | (msb << 8);

    if (length == 0) break;
    if (tapFile.available() < length) {
      Serial.println("Warning: Incomplete block");
      break;
    }

    byte* buffer = (byte*)malloc(length);
    if (!buffer) {
      Serial.println("Out of memory");
      break;
    }
    int readLen = tapFile.read(buffer, (size_t)length);
    if (readLen != length) {
       Serial.println("Read error");
    }

    Serial.print("Sending block, size: ");
    Serial.print(length);
    Serial.print(" Flag: 0x");
    Serial.println(buffer[0], HEX);

    modem.sendRawBlock(buffer, length);
    free(buffer);
  }

  tapFile.close();
  if (!selectMode) {
    Serial.println("Done.");
  }
}
