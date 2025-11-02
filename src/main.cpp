//! THIS IS A WORK IN PROGRESS
//! DO NOT USE FOR TESTING!

//! 2025-08-26

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
 * - TX_PIN:  Audio Frequency Transmit.
 * - RX_PIN:  Audio Frequency Receive (used for analog input).
 * - PTT_PIN: Push-to-Talk control pin.
 * - PTT_LED: LED to indicate PTT status.
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
 * @author 2E0UMR, adapted to PlatformIO by W4KRL
 * @see https://uhpowerup.com/
 * @see https://hackaday.io/project/202603-esp32-kiss-tnc
 * @see https://github.com/rkinnett/Esp32-Bluetooth-KISS-Demo
 */

#include <Arduino.h>        // Include the Arduino core for ESP32
#include "configuration.h"  // Include configuration settings
#include "btFunctions.h"    // Include Bluetooth functions
#include "afskEncoder.h"    // Include modern AFSK encoder functions
#include "afskDecode.h"     // Include AFSK demodulation functions
#include "wifiConnection.h" // Include WiFi connection functions
#include "ArduinoOTA.h"     // Include OTA update functions

// Test pattern selection - change this to select different test patterns
typedef enum {
  TEST_CONTINUOUS_MARK,     // Constant 1200 Hz (all 1s)
  TEST_CONTINUOUS_SPACE,    // Constant 2200 Hz (all 0s)
  TEST_ALTERNATING,         // Alternating 1200/2200 Hz (1,0,1,0...)
  TEST_SLOW_ALTERNATING     // Slow alternating (1 second mark, 1 second space)
} test_pattern_t;

#define ENABLE_AFSK_TEST  
#define CURRENT_TEST_PATTERN TEST_CONTINUOUS_SPACE  // <-- Change this line to select test pattern

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
  delay(1000); // Allow serial to initialize
  
  Serial.println("\n=== ESP32 KISS TNC Starting ===");
  
  setupBluetooth();     // Initialize Bluetooth Serial
  wifiBegin();          // Setup WiFi
  wifiConnect();        // Connect to WiFi
  ArduinoOTA.begin();   // Initialize OTA updates
  
  // Initialize the new function-based AFSK encoder
  afsk_status_t status = setupAFSKEncoder();
  if (status == AFSK_SUCCESS) {
    Serial.println("AFSK encoder initialized successfully");
  } else {
    Serial.print("AFSK encoder failed: ");
    Serial.println(getAFSKStatusString(status));
  }
  
  setupAFSKdecoder();   // Initialize Goertzel filter for AFSK demodulation
}

/**
 * @brief AFSK Test Function - sends different test patterns
 */
void runAFSKTest() {
  static bool testStarted = false;
  static unsigned long lastTransmission = 0;
  static bool slowAlternatingState = true; // For slow alternating test
  
  // Print test info once
  if (!testStarted) {
    Serial.println("\n=== AFSK Test Mode Started ===");
    switch (CURRENT_TEST_PATTERN) {
      case TEST_CONTINUOUS_MARK:
        Serial.println("Test: CONTINUOUS MARK (1200 Hz)");
        break;
      case TEST_CONTINUOUS_SPACE:
        Serial.println("Test: CONTINUOUS SPACE (2200 Hz)");
        break;
      case TEST_ALTERNATING:
        Serial.println("Test: FAST ALTERNATING (1200/2200 Hz per bit)");
        break;
      case TEST_SLOW_ALTERNATING:
        Serial.println("Test: SLOW ALTERNATING (1 sec mark, 1 sec space)");
        break;
    }
    Serial.println("*** ALL OTHER PROCESSES BYPASSED ***");
    testStarted = true;
  }
  
  // Test parameters
  const int BITS_PER_TRANSMISSION = 1200; // 1 second worth at 1200 baud
  static uint8_t testBits[BITS_PER_TRANSMISSION];
  static bool bitsInitialized = false;
  
  // Initialize bit patterns
  if (!bitsInitialized) {
    switch (CURRENT_TEST_PATTERN) {
      case TEST_CONTINUOUS_MARK:
        for (int i = 0; i < BITS_PER_TRANSMISSION; i++) {
          testBits[i] = 1; // All marks (1200 Hz)
        }
        break;
        
      case TEST_CONTINUOUS_SPACE:
        for (int i = 0; i < BITS_PER_TRANSMISSION; i++) {
          testBits[i] = 0; // All spaces (2200 Hz)
        }
        break;
        
      case TEST_ALTERNATING:
        for (int i = 0; i < BITS_PER_TRANSMISSION; i++) {
          testBits[i] = i % 2; // Alternating 0,1,0,1...
        }
        break;
        
      case TEST_SLOW_ALTERNATING:
        // Will be set dynamically in the loop
        break;
    }
    bitsInitialized = true;
  }
  
  // Send test patterns
  switch (CURRENT_TEST_PATTERN) {
    case TEST_CONTINUOUS_MARK:
    case TEST_CONTINUOUS_SPACE:
    case TEST_ALTERNATING:
      // Continuous transmission with 3 second intervals to reduce debug spam
      if (millis() - lastTransmission > 3000) {
        afskSend(testBits, BITS_PER_TRANSMISSION);
        lastTransmission = millis();
      }
      break;
      
    case TEST_SLOW_ALTERNATING:
      // Send 1 second of mark, then 1 second of space
      if (millis() - lastTransmission > 1000) {
        if (slowAlternatingState) {
          Serial.println("Sending 1 second MARK (1200 Hz)...");
          for (int i = 0; i < BITS_PER_TRANSMISSION; i++) {
            testBits[i] = 1; // Mark
          }
        } else {
          Serial.println("Sending 1 second SPACE (2200 Hz)...");
          for (int i = 0; i < BITS_PER_TRANSMISSION; i++) {
            testBits[i] = 0; // Space
          }
        }
        
        afskSend(testBits, BITS_PER_TRANSMISSION);
        slowAlternatingState = !slowAlternatingState;
        lastTransmission = millis();
      }
      break;
  }
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
  if (ENABLE_AFSK_TEST) {
    // Run the configurable AFSK test
    runAFSKTest();
    delay(10); // Small delay to prevent watchdog reset
  } else {
    // Normal operation mode
    wifiConnect();       // Reconnect to Wi-Fi if disconnected
    ArduinoOTA.handle(); // Check for OTA updates
    checkBTforData(); // Check Bluetooth Serial for incoming data
    receiveAFSK();    // Decode AFSK
  }
}
