
#ifndef SIMPLE_FSK_MODEM_H
#define SIMPLE_FSK_MODEM_H

#include <Arduino.h>

// Define some constants for the FSK parameters
#define FSK_MARK_FREQ 1200 // Mark frequency in Hz
#define FSK_SPACE_FREQ 2200 // Space frequency in Hz
#define FSK_BAUD_RATE 300 // Baud rate in bits per second
#define FSK_BIT_DURATION (1000 / FSK_BAUD_RATE) // Bit duration in milliseconds

// Define some constants for the audio output
#define FSK_AUDIO_PIN 9 // Pin number for audio output
#define FSK_AUDIO_AMP 127 // Amplitude of audio signal (0-255)

// Define a class for the SimpleFSKModem library
class SimpleFSKModem {
  public:
    // Constructor that takes an optional pin number for audio output
    SimpleFSKModem(int pin = FSK_AUDIO_PIN);

    // Method to initialize the library
    void begin();

    // Method to send a byte of data using FSK modulation
    void sendByte(byte data);

    // Method to send an array of bytes of data using FSK modulation
    void sendBytes(byte* data, int length);

    // Method to send a string of data using FSK modulation
    void sendString(String data);

  private:
    // Variable to store the pin number for audio output
    int _pin;

    // Method to generate a tone of a given frequency and duration on the audio output pin
    void tone(int freq, int duration);
};

#endif

//Source: Conversation with Bing, 4/26/2023
//(1) Writing a Library for Arduino | Arduino Documentation. https://www.arduino.cc/en/Hacking/LibraryTutorial.
//(2) HackerBox 0090: Modem : 6 Steps - Instructables. https://www.instructables.com/HackerBox-0090-Modem/.
//(3) Best Open Source Assembly Communications Software - SourceForge. https://sourceforge.net/directory/communications/assembly/.
//(4) pschatzmann/arduino-codec2 - zesizon.afphila.com. https://zesizon.afphila.com/pschatzmann/arduino-codec2.
//(5) Create Your Own Arduino Library - The Robotics Back-End. https://roboticsbackend.com/arduino-create-library/.
