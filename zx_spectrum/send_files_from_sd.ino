#include <SD.h>
#include <SimpleFSKModem.h>

// Define some constants for the SD card
#define SD_CS_PIN 10
#define FILE_ORDER_LIST "list.txt"

// Define some constants for the FSK modem
#define FSK_AUDIO_PIN 9

// Define some constants for the buttons
#define FIRST_BUTTON_PIN 2
#define SECOND_BUTTON_PIN 3
#define THIRD_BUTTON_PIN 4
#define FOURTH_BUTTON_PIN 5
#define FIFTH_BUTTON_PIN 6
#define SIXTH_BUTTON_PIN 7

// Define some constants for the delay knob
#define DELAY_KNOB_PIN A0
#define DELAY_MIN 1000
#define DELAY_MAX 10000

SimpleFSKModem modem(FSK_AUDIO_PIN);
String currentFile = "";
int currentFileIndex = -1;
int numFiles = 0;
bool sendingFile = false;
bool repeatingFile = false;

// Function prototypes
int readFileOrderList();
void selectFirstFile();
void selectNextFile();
void selectPreviousFile();
void selectLastFile();
void sendFile(String fileName);
void sendCurrentFile();
void handleFirstButton();
void handleSecondButton();
void handleThirdButton();
void handleFourthButton();
void handleFifthButton();
void handleSixthButton();

void setup() {
  Serial.begin(115200);
  modem.begin();
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialization done.");

  pinMode(FIRST_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SECOND_BUTTON_PIN, INPUT_PULLUP);
  pinMode(THIRD_BUTTON_PIN, INPUT_PULLUP);
  pinMode(FOURTH_BUTTON_PIN, INPUT_PULLUP);
  pinMode(FIFTH_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SIXTH_BUTTON_PIN, INPUT_PULLUP);

  numFiles = readFileOrderList();
  Serial.print("Number of files in list: ");
  Serial.println(numFiles);

  if (numFiles > 0) {
    selectFirstFile();
  }
}

void loop() {
  if (digitalRead(FIRST_BUTTON_PIN) == LOW) {
    handleFirstButton();
    delay(500);
  }

  if (digitalRead(SECOND_BUTTON_PIN) == LOW) {
    handleSecondButton();
    delay(500);
  }

  if (digitalRead(THIRD_BUTTON_PIN) == LOW) {
    handleThirdButton();
    delay(500);
  }

  if (digitalRead(FOURTH_BUTTON_PIN) == LOW) {
    handleFourthButton();
    delay(500);
  }

  if (digitalRead(FIFTH_BUTTON_PIN) == LOW) {
    handleFifthButton();
    delay(500);
  }

  if (digitalRead(SIXTH_BUTTON_PIN) == LOW) {
    handleSixthButton();
    delay(500);
  }

  if (sendingFile) {
    sendCurrentFile();
    if (repeatingFile) {
      int delayTime = map(analogRead(DELAY_KNOB_PIN), 0, 1023, DELAY_MIN, DELAY_MAX);
      Serial.print("Waiting for ");
      Serial.print(delayTime);
      Serial.println(" milliseconds");
      delay(delayTime);
    } else {
      sendingFile = false;
    }
    selectNextFile();
    Serial.print("Selected next file: ");
    Serial.println(currentFile);
  }
}

int readFileOrderList() {
  File list = SD.open(FILE_ORDER_LIST, FILE_READ);
  if (!list) {
    Serial.println("Failed to open list.txt!");
    return -1;
  }
  int count = 0;
  while (list.available()) {
    String line = list.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      count++;
      Serial.print(count);
      Serial.print(": ");
      Serial.println(line);
    }
  }
  list.close();
  return count;
}

void selectFirstFile() {
  File list = SD.open(FILE_ORDER_LIST, FILE_READ);
  if (!list) {
    Serial.println("Failed to open list.txt!");
    return;
  }
  while (list.available()) {
    String line = list.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      currentFile = line;
      currentFileIndex = 0;
      break;
    }
  }
  list.close();
}

void selectNextFile() {
  File list = SD.open(FILE_ORDER_LIST, FILE_READ);
  if (!list) {
    Serial.println("Failed to open list.txt!");
    return;
  }
  int index = -1;
  bool found = false;
  while (list.available()) {
    String line = list.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      index++;
      if (index == currentFileIndex + 1) {
        currentFile = line;
        currentFileIndex = index;
        found = true;
        break;
      }
    }
  }
  list.close();
  if (!found) {
    selectFirstFile();
  }
}

void selectPreviousFile() {
  if (currentFileIndex <= 0) {
    selectLastFile();
    return;
  }

  File list = SD.open(FILE_ORDER_LIST, FILE_READ);
  if (!list) {
    Serial.println("Failed to open list.txt!");
    return;
  }
  int index = -1;
  int targetIndex = currentFileIndex - 1;
  while (list.available()) {
    String line = list.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      index++;
      if (index == targetIndex) {
        currentFile = line;
        currentFileIndex = index;
        break;
      }
    }
  }
  list.close();
}

void selectLastFile() {
  File list = SD.open(FILE_ORDER_LIST, FILE_READ);
  if (!list) {
    Serial.println("Failed to open list.txt!");
    return;
  }
  int index = -1;
  String lastFile = "";
  while (list.available()) {
    String line = list.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      index++;
      lastFile = line;
    }
  }
  if (index != -1) {
    currentFile = lastFile;
    currentFileIndex = index;
  }
  list.close();
}

void sendFile(String fileName) {
  File data = SD.open(fileName.c_str(), FILE_READ);
  if (!data) {
    Serial.println("Failed to open data file: " + fileName);
    return;
  }
  Serial.println("Sending file: " + fileName);
  while (data.available()) {
    byte b = data.read();
    modem.sendByte(b);
  }
  data.close();
}

void sendCurrentFile() {
  if (currentFile != "") {
    sendFile(currentFile);
  }
}

void handleFirstButton() {
  selectFirstFile();
  Serial.print("Selected first file: ");
  Serial.println(currentFile);
}

void handleSecondButton() {
  sendingFile = true;
  repeatingFile = false;
}

void handleThirdButton() {
  sendingFile = true;
  repeatingFile = true;
}

void handleFourthButton() {
  sendingFile = false;
  repeatingFile = false;
  Serial.println("Stopped sending file.");
}

void handleFifthButton() {
  selectPreviousFile();
  Serial.print("Selected previous file: ");
  Serial.println(currentFile);
}

void handleSixthButton() {
  selectNextFile();
  Serial.print("Selected next file: ");
  Serial.println(currentFile);
}
