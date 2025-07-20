/**
 * @file configuration.h
 * @brief Configuration constants for ESP32 KISS TNC project.
 *
 * This header defines Bluetooth device name, and pin assignments
 * for the ESP32-based KISS TNC (Terminal Node Controller).
 *
 * Pin Definitions:
 * - PTT_PIN: GPIO pin used for Push-to-Talk (PTT) control.
 * - PTT_LED: GPIO pin connected to an LED indicating PTT status.
 * - TX_PIN:  GPIO pin for transmitting audio to the radio.
 * - RX_PIN:  GPIO pin for receiving audio from the radio.
 * 
 * @note There is no matching .cpp file for this header
 *	Add the following to platformio.ini
 *		build_unflags = -std=gnu++11
 *		build_flags = -std=gnu++17
 *  This allows the use of inline variables in this header file.
 *  See https://docs.platformio.org/en/latest/projectconf/advanced_syntax.html#build-flags
 *  for more information on build flags.
 */
#include <Arduino.h> // for IPAddress

// Bluetooth device name for the KISS TNC
inline const char *BT_NAME = "ESP32 KISS TNC";

// Pin definitions for transceiver interface
inline const int PTT_PIN = 4;			// Push-to-Talk control pin
inline const int PTT_LED = LED_BUILTIN; // LED to indicate PTT status
inline const int TX_PIN = 25;			// Audio to radio
inline const int RX_PIN = 34;			// Audio from radio