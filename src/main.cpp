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
 * - TX_PIN: Audio Frequency Transmit (not used in this example).
 * - RX_PIN: Audio Frequency Receive (used for analog input and tone output).
 * - PTT_PIN: Push-to-Talk control pin.
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

#include <Arduino.h>       // Include the Arduino core for ESP32
#include "configuration.h" // Include configuration settings
#include "btFunctions.h"   // Include Bluetooth functions
#include "afskEncode.h"    // Include AFSK modulation functions
#include "afskDecode.h"    // Include AFSK demodulation functions

/**
 * @brief Initializes the ESP32 KISS TNC.
 *
 * This function sets up the necessary components for the KISS TNC:
 * - Initializes USB Serial communication for debugging.
 * - Sets up Bluetooth Serial communication.
 * - Configures AFSK modulation settings.
 * - Initializes the Goertzel filter for AFSK demodulation.
 */
void setup()
{
  Serial.begin(115200); // USB Serial for debugging
  setupBluetooth();     // Initialize Bluetooth Serial
  setupAFSKencoder();   // Initialize AFSK modulation settings
  setupAFSKdecoder();   // Initialize Goertzel filter for AFSK demodulation
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
  checkBTforData(); // Check Bluetooth Serial for incoming data
  receiveAFSK();    // Decode AFSK
}
