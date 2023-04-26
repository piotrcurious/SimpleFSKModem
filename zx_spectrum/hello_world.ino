#include <SimpleFSKModem.h>

// Create an instance of the SimpleFSKModem class using pin 9 for audio output
SimpleFSKModem modem(9);

void setup() {
  modem.begin(); // Initialize the modem
}

void loop() {
  modem.sendString("Hello world!"); // Send a string using ZX-Spectrum compatible FSK modulation
  delay(5000); // Wait for 5 seconds
}
