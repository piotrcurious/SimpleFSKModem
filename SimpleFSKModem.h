#ifndef SIMPLE_FSK_MODEM_H
#define SIMPLE_FSK_MODEM_H

#include <Arduino.h>

// Default FSK parameters
#define DEFAULT_FSK_MARK_FREQ 1200 // Mark frequency in Hz
#define DEFAULT_FSK_SPACE_FREQ 2200 // Space frequency in Hz
#define DEFAULT_FSK_BAUD_RATE 300 // Baud rate in bits per second

// Define some constants for the audio output
#define FSK_AUDIO_PIN 9 // Pin number for audio output
#define FSK_AUDIO_AMP 127 // Amplitude of audio signal (0-255)

// Define a class for the SimpleFSKModem library
class SimpleFSKModem {
  public:
    // Constructor that takes an optional pin number for audio output
    SimpleFSKModem(int pin = FSK_AUDIO_PIN);

    // Method to set FSK parameters
    void setParameters(int markFreq, int spaceFreq, int baudRate);

    // Method to initialize the library
    void begin();

    // Method to send a byte of data using FSK modulation
    void sendByte(byte data);

    // Method to send an array of bytes of data using FSK modulation
    void sendBytes(byte* data, int length);

    // Method to send a string of data using FSK modulation
    void sendString(String data);

    // Method to generate a tone of a given frequency and duration on the audio output pin
    void tone(int freq, uint32_t duration_us);

  private:
    int _pin;
    int _markFreq;
    int _spaceFreq;
    int _baudRate;
    uint32_t _bitDurationUs;

#ifdef ARDUINO_ARCH_AVR
    volatile uint8_t *_portReg;
    uint8_t _pinMask;
#endif
};

#endif
