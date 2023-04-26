//Bonjour, this is Bing. I can help you create an example using the library to send all files from SD card, basing on file order list list.txt. First, you need to include the SD card library and the SimpleFSKModem library in your sketch. Then, you need to define some constants and variables for the SD card, the FSK modem, and the buttons. Next, you need to initialize the SD card and the FSK modem in the setup() function. You also need to set the pins for the buttons as inputs with pull-up resistors. After that, you need to write some functions to read the file order list, select a file, send a file, and handle the button actions. Finally, you need to call these functions in the loop() function according to the button states.

//Here is an example of how you can write such a sketch:

#include <SD.h> // Include the SD card library
#include <SimpleFSKModem.h> // Include the SimpleFSKModem library

// Define some constants for the SD card
#define SD_CS_PIN 10 // Chip select pin for SD card
#define FILE_ORDER_LIST "list.txt" // File name for file order list

// Define some constants for the FSK modem
#define FSK_AUDIO_PIN 9 // Pin number for audio output

// Define some constants for the buttons
#define FIRST_BUTTON_PIN 2 // Pin number for first button (select first file)
#define SECOND_BUTTON_PIN 3 // Pin number for second button (send current file, select next file and stop)
#define THIRD_BUTTON_PIN 4 // Pin number for third button (send current file and repeat after delay)
#define FOURTH_BUTTON_PIN 5 // Pin number for fourth button (stop sending file)
#define FIFTH_BUTTON_PIN 6 // Pin number for fifth button (select previous file)
#define SIXTH_BUTTON_PIN 7 // Pin number for sixth button (select next file)

// Define some constants for the delay knob
#define DELAY_KNOB_PIN A0 // Pin number for delay knob
#define DELAY_MIN 1000 // Minimum delay in milliseconds
#define DELAY_MAX 10000 // Maximum delay in milliseconds

// Create an instance of the SimpleFSKModem class using pin 9 for audio output
SimpleFSKModem modem(FSK_AUDIO_PIN);

// Create a variable to store the current file name
String currentFile = "";

// Create a variable to store the current file index
int currentFileIndex = -1;

// Create a variable to store the number of files in the list
int numFiles = 0;

// Create a variable to store the state of sending file
bool sendingFile = false;

// Create a variable to store the state of repeating file
bool repeatingFile = false;

void setup() {
  Serial.begin(115200); // Initialize serial communication
  modem.begin(); // Initialize FSK modem
  if (!SD.begin(SD_CS_PIN)) { // Initialize SD card
    Serial.println("SD card initialization failed!"); // Print error message if failed
    return; // Stop execution
  }
  Serial.println("SD card initialization done."); // Print success message if done

  pinMode(FIRST_BUTTON_PIN, INPUT_PULLUP); // Set first button pin as input with pull-up resistor
  pinMode(SECOND_BUTTON_PIN, INPUT_PULLUP); // Set second button pin as input with pull-up resistor
  pinMode(THIRD_BUTTON_PIN, INPUT_PULLUP); // Set third button pin as input with pull-up resistor
  pinMode(FOURTH_BUTTON_PIN, INPUT_PULLUP); // Set fourth button pin as input with pull-up resistor
  pinMode(FIFTH_BUTTON_PIN, INPUT_PULLUP); // Set fifth button pin as input with pull-up resistor
  pinMode(SIXTH_BUTTON_PIN, INPUT_PULLUP); // Set sixth button pin as input with pull-up resistor

  numFiles = readFileOrderList(); // Read the file order list and get the number of files
  Serial.print("Number of files in list: "); // Print message
  Serial.println(numFiles); // Print number of files

}

void loop() {
  if (digitalRead(FIRST_BUTTON_PIN) == LOW) { // Check if first button is pressed
    selectFirstFile(); // Select first file in list
    Serial.print("Selected first file: "); // Print message
    Serial.println(currentFile); // Print current file name
    delay(500); // Wait for half a second to avoid bouncing
  }

  if (digitalRead(SECOND_BUTTON_PIN) == LOW) { // Check if second button is pressed
    sendCurrentFile(); // Send current file using FSK modulation
    Serial.print("Sent current file: "); // Print message
    Serial.println(currentFile); // Print current file name
    selectNextFile(); // Select next file in list
    Serial.print("Selected next file: "); // Print message
    Serial.println(currentFile); // Print current file name
    sendingFile = false; // Set sending file state to false
    repeatingFile = false; // Set repeating file state to false
    delay(500); // Wait for half a second to avoid bouncing
  }

  if (digitalRead(THIRD_BUTTON_PIN) == LOW) { // Check if third button is pressed
    sendCurrentFile(); // Send current file using FSK modulation
    Serial.print("Sent current file: "); // Print message
    Serial.println(currentFile); // Print current file name
    int delayTime = map(analogRead(DELAY_KNOB_PIN), 0, 1023, DELAY_MIN, DELAY_MAX); // Map the delay knob value to a delay time between minimum and maximum
    Serial.print("Waiting for "); // Print message
    Serial.print(delayTime); // Print delay time
    Serial.println(" milliseconds"); // Print message
    delay(delayTime); // Wait for the delay time
    selectNextFile(); // Select next file in list
    Serial.print("Selected next file: "); // Print message
    Serial.println(currentFile); // Print current file name
    sendingFile = true; // Set sending file state to true
    repeatingFile = true; // Set repeating file state to true
    delay(500); // Wait for half a second to avoid bouncing
  }

  if (digitalRead(FOURTH_BUTTON_PIN) == LOW) { // Check if fourth button is pressed
    sendingFile = false; // Set sending file state to false
    repeatingFile = false; // Set repeating file state to false
    Serial.println("Stopped sending file."); // Print message
    delay(500); // Wait for half a second to avoid bouncing
  }

  if (digitalRead(FIFTH_BUTTON_PIN) == LOW) { // Check if fifth button is pressed
    selectPreviousFile(); // Select previous file in list
    Serial.print("Selected previous file: "); // Print message
    Serial.println(currentFile); // Print current file name
    delay(500); // Wait for half a second to avoid bouncing
  }

  if (digitalRead(SIXTH_BUTTON_PIN) == LOW) { // Check if sixth button is pressed
    selectNextFile(); // Select next file in list
    Serial.print("Selected next file: "); // Print message
    Serial.println(currentFile); // Print current file name
    delay(500); // Wait for half a second to avoid bouncing
  }

  if (sendingFile && !repeatingFile) { // Check if sending file and not repeating file states are true
    sendCurrentFile(); // Send current file using FSK modulation
    Serial.print("Sent current file: "); // Print message
    Serial.println(currentFile); // Print current file name
    selectNextFile(); // Select next file in list
    Serial.print("Selected next file: "); // Print message
    Serial.println(currentFile); // Print current file name
  }
}

// Function to read the file order list and get the number of files in it 
int readFileOrderList() {
  File list = SD.open(FILE_ORDER_LIST, FILE_READ); // Open the list.txt file for reading 
  if (!list) { // Check if the file opening failed 
    Serial.println("Failed to open list.txt!"); // Print error message 
    return -1; // Return an error code (-1)
  }
  int count = 0; // Initialize a counter to store the number of files 
  while (list.available()) { // Loop while there is data available in the file 
    String line = list.readStringUntil('\n'); // Read a line from the file until a newline character 
    line.trim(); // Remove any leading or trailing whitespace from the line 
    if (line.length() > 0) { // Check if the line is not empty 
      count++; // Increment the counter 
      Serial.print(count);

  int readFileOrderList() {
    File list = SD.open(FILE_ORDER_LIST, FILE_READ); // Open the list.txt file for reading 
    if (!list) { // Check if the file opening failed 
      Serial.println("Failed to open list.txt!"); // Print error message 
      return -1; // Return an error code (-1)
    }
    int count = 0; // Initialize a counter to store the number of files 
    while (list.available()) { // Loop while there is data available in the file 
      String line = list.readStringUntil('\n'); // Read a line from the file until a newline character 
      line.trim(); // Remove any leading or trailing whitespace from the line 
      if (line.length() > 0) { // Check if the line is not empty 
        count++; // Increment the counter 
        Serial.print(count); // Print the file number 
        Serial.print(": "); // Print a separator 
        Serial.println(line); // Print the file name 
      }
    }
    list.close(); // Close the file 
    return count; // Return the number of files 
  }

  // Function to select the first file in the list
  void selectFirstFile() {
    File list = SD.open(FILE_ORDER_LIST, FILE_READ); // Open the list.txt file for reading
    if (!list) { // Check if the file opening failed
      Serial.println("Failed to open list.txt!"); // Print error message
      return; // Stop execution
    }
    while (list.available()) { // Loop while there is data available in the file
      String line = list.readStringUntil('\n'); // Read a line from the file until a newline character
      line.trim(); // Remove any leading or trailing whitespace from the line
      if (line.length() > 0) { // Check if the line is not empty
        currentFile = line; // Set the current file name to the line
        currentFileIndex = 0; // Set the current file index to 0
        break; // Break the loop
      }
    }
    list.close(); // Close the file
  }

  // Function to select the next file in the list
  void selectNextFile() {
    File list = SD.open(FILE_ORDER_LIST, FILE_READ); // Open the list.txt file for reading
    if (!list) { // Check if the file opening failed
      Serial.println("Failed to open list.txt!"); // Print error message
      return; // Stop execution
    }
    int index = -1; // Initialize an index variable to -1
    while (list.available()) { // Loop while there is data available in the file
      String line = list.readStringUntil('\n'); // Read a line from the file until a newline character
      line.trim(); // Remove any leading or trailing whitespace from the line
      if (line.length() > 0) { // Check if the line is not empty
        index++; // Increment the index variable
        if (index == currentFileIndex + 1) { // Check if the index is one more than the current file index
          currentFile = line; // Set the current file name to the line
          currentFileIndex = index; // Set the current file index to the index
          break; // Break the loop
        }
      }
    }
    if (index == -1 || index < currentFileIndex + 1) { // Check if no valid index was found or reached end of list
      selectFirstFile(); // Select first file in list
    }
    list.close(); // Close the file
  }

  // Function to select the previous file in the list
  void selectPreviousFile() {
    File list = SD.open(FILE_ORDER_LIST, FILE_READ); // Open the list.txt file for reading
    if (!list) { // Check if the file opening failed
      Serial.println("Failed to open list.txt!"); // Print error message
      return; // Stop execution
    }
    int index = -1; // Initialize an index variable to -1
    String previousFile = ""; // Initialize a string variable to store the previous file name
    while (list.available()) { // Loop while there is data available in the file
      String line = list.readStringUntil('\n'); // Read a line from the file until a newline character
      line.trim(); // Remove any leading or trailing whitespace from the line
      if (line.length() > 0) { // Check if the line is not empty
        index++; // Increment the index variable
        if (index == currentFileIndex - 1) { // Check if the index is one less than the current file index
          previousFile = line; // Set the previous file name to the line
          break; // Break the loop
        }
      }
    }
    if (index == -1 || previousFile == "") { // Check if no valid index was found or no previous file was found
      selectLastFile(); // Select last file in list
    } else {
      currentFile = previousFile; // Set the current file name to the previous file name
      currentFileIndex = index; // Set the current file index to the index
    }
    list.close(); // Close the file
  }

  // Function to select the last file in the list
  void selectLastFile() {
    File list = SD.open(FILE_ORDER_LIST, FILE_READ); // Open the list.txt file for reading
    if (!list) { // Check if the file opening failed
      Serial.println("Failed to open list.txt!"); // Print error message
      return; // Stop execution
    }
    int index = -1; // Initialize an index variable to -1
    String lastFile = ""; // Initialize a string variable to store the last file name
    while (list.available()) { // Loop while there is data available in the file
      String line = list.readStringUntil('\n'); // Read a line from the file until a newline character
      line.trim(); // Remove any leading or trailing whitespace from the line
      if (line.length() > 0) { // Check if the line is not empty
        index++; // Increment the index variable
        lastFile = line; // Set the last file name to the line
      }
    }
    currentFile = lastFile; // Set the current file name to the last file name
    currentFileIndex = index; // Set the current file index to the index 
    list.close(); // Close the file 
  }

  // Function to send a file using FSK modulation 
  void sendFile(String fileName) {
    File data = SD.open(fileName, FILE_READ); // Open the data file for reading 
    if (!data) { // Check if the file opening failed 
      Serial.println("Failed to open data file!"); // Print error message 
      return; // Stop execution 
    }
    while (data.available()) { // Loop while there is data available in the file 
      byte b = data.read(); // Read a byte from the file 
      modem.sendByte(b); // Send a byte using FSK modulation 
    }
    data.close(); // Close the file 
  }

  // Function to send current file using FSK modulation 
  void sendCurrentFile() {
    sendFile(currentFile); // Send current file using FSK modulation 
  }

  // Function to handle first button action (select first file)
  void handleFirstButton() {
    selectFirstFile(); // Select first file in list
    Serial.print("Selected first file: "); // Print message
    Serial.println(currentFile); // Print current file name
  }

  // Function to handle second button action (send current file, select next file and stop)
  void handleSecondButton() {
    sendCurrentFile(); // Send current file using FSK modulation
    Serial.print("Sent current file: "); // Print message
    Serial.println(currentFile); // Print current file name
    selectNextFile(); // Select next file in list
    Serial.print("Selected next file: "); // Print message
    Serial.println(currentFile); // Print current file name
    sendingFile = false; // Set sending file state to false
    repeatingFile = false; // Set repeating file state to false
  }

  // Function to handle third button action (send current file and repeat after delay)
  void handleThirdButton() {
    sendCurrentFile(); // Send current file using FSK modulation
    Serial.print("Sent current file: "); // Print message
    Serial.println(currentFile); // Print current file name
    int delayTime = map(analogRead(DELAY_KNOB_PIN), 0, 1023, DELAY_MIN, DELAY_MAX); // Map the delay knob value to a delay time between minimum and maximum
    Serial.print("Waiting for "); // Print message
    Serial.print(delayTime); // Print delay time
    Serial.println(" milliseconds"); // Print message
    delay(delayTime); // Wait for the delay time
    selectNextFile(); // Select next file in list
    Serial.print("Selected next file: "); // Print message
    Serial.println(currentFile); // Print current file name
    sendingFile = true; // Set sending file state to true
    repeatingFile = true; // Set repeating file state to true
  }

  // Function to handle fourth button action (stop sending file)
  void handleFourthButton() {
    sendingFile = false; // Set sending file state to false
    repeatingFile = false; // Set repeating file state to false
    Serial.println("Stopped sending file."); // Print message
  }

  // Function to handle fifth button action (select previous file)
  void handleFifthButton() {
    selectPreviousFile(); // Select previous file in list
    Serial.print("Selected previous file: "); // Print message
    Serial.println(currentFile); // Print current file name
  }

  // Function to handle sixth button action (select next file)
  void handleSixthButton() {
    selectNextFile(); // Select next file in list 
    Serial.print("Selected next file: "); // Print message 
    Serial.println(currentFile); // Print current file name 
  }

//Source: Conversation with Bing, 4/26/2023
//(1) SD - Arduino Reference. https://reference.arduino.cc/reference/en/libraries/sd/.
//(2) GitHub - arduino-libraries/SD: SD Library for Arduino. https://github.com/arduino-libraries/SD.
//(3) SD - Arduino Reference. https://www.arduino.cc/en/Reference/SD.
