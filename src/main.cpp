//! THIS IS A WORK IN PROGRESS
//! DO NOT USE FOR TESTING!

/**
 * @file main.cpp
 * @brief ESP32 KISS TNC implementation with Bluetooth and USB Serial support.
 *
 * This program implements a KISS (Keep It Simple Stupid) TNC (Terminal Node Controller)
 * for the ESP32 platform, supporting both USB Serial and Bluetooth Serial interfaces.
 * It is designed for amateur radio applications, enabling packet radio communication
 * using the KISS protocol over AFSK (Audio Frequency Shift Keying) modulation.
 *
 * Features:
 * - Sends and receives KISS frames over USB Serial and Bluetooth Serial.
 * - Encodes and decodes KISS special characters (FEND, FESC, TFEND, TFESC).
 * - Generates AFSK tones for packet transmission (simplified example).
 * - Detects incoming AFSK audio and sends decoded packets as KISS frames (simplified).
 * - Controls PTT (Push-to-Talk) for radio transmission.
 *
 * Pin Definitions:
 * - TX_PIN (25): Audio Frequency Transmit (not used in this example).
 * - RX_PIN (34): Audio Frequency Receive (used for analog input and tone output).
 * - PTT_PIN (13): Push-to-Talk control pin.
 *
 * Usage:
 * - Connect the ESP32 to a radio transceiver using the defined pins.
 * - Communicate with the TNC using a terminal or application over USB or Bluetooth.
 * - Use KISS protocol frames for packet radio communication.
 *
 * Notes:
 * - The AFSK modulation/demodulation is highly simplified and intended for demonstration.
 * - Proper AFSK implementation requires more advanced signal processing.
 * - Use at your own risk; modifications may be required for specific hardware setups.
 *
 * @author AI generated code modified by 2E0UMR, adapted to PlatformIO by W4KRL
 * @see https://uhpowerup.com/
 * @see https://hackaday.io/project/202603-esp32-kiss-tnc
 * @see https://github.com/rkinnett/Esp32-Bluetooth-KISS-Demo
 */

#include <Arduino.h>        // Include the Arduino core for ESP32
#include "configuration.h"  // Include configuration settings (WiFi, IP, etc.)
#include "btFunctions.h"    // Implement Bluetooth functions
#include "afskFunctions.h"  // Include AFSK modulation functions
#include "kissFunctions.h"  // Include KISS protocol functions
#include "wifiConnection.h" // Include WiFi and OTA connection management functions
#include "ArduinoOTA.h"     // for loop() OTA update
#include "goertzelFilter.h" // Include Goertzel filter for AFSK demodulation
#include "driver/ledc.h"

/**
 * @brief Set up function initializes serial communication and Bluetooth.
 *
 * This function sets up the USB serial port for debugging at 115200 baud
 * and calls the function to initialize Bluetooth serial communication.
 */
void setup()
{
  Serial.begin(115200); // USB Serial for debugging
  wifiBegin();          // Setup WiFi
  wifiConnect();        // Connect to WiFi
  otaBegin();           // Initialize Over-The-Air update service
  setupAFSK();          // Initialize AFSK modulation settings
  setupBluetooth();     // Initialize Bluetooth Serial

  analogReadResolution(12);
  setupGoertzel(); // Initialize Goertzel filter for AFSK demodulation
}

/**
 * @brief Main loop function for handling KISS frames and AFSK processing.
 *
 * This function continuously checks for incoming KISS frames on the
 * Bluetooth Serial interface. When data is available, it reads the frame and
 * transmits it using AFSK modulation. Additionally, it processes any incoming audio
 * signals by calling receiveAFSK().
 *
 * - Checks Bluetooth Serial for available KISS frames and transmits them via AFSK.
 * - Continuously processes incoming audio for AFSK reception.
 */
void loop()
{
  wifiConnect();     // Reconnect to Wi-Fi if disconnected
  checkBTforData();  // Check Bluetooth Serial for incoming data
  processGoertzel(); // Decode AFSK
}
