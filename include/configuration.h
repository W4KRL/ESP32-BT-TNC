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
 * - TX_PIN:  Uses GPIO25 set as DAC_CHANNEL_1 in afskEncode.cpp for AFSK audio output.
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
#define BT_NAME "ESP32 KISS TNC"

// WiFi Credentials
inline const char *WIFI_SSID = "DCMNET";
inline const char *WIFI_PASSWORD = "0F1A2D3E4D5G6L7O8R9Y";

// Static IP Settings
inline const IPAddress LOCAL_IP(192, 168, 0, 234);
inline const IPAddress GATEWAY(192, 168, 0, 1);
inline const IPAddress SUBNET(255, 255, 255, 0);

// Pin definitions for transceiver interface
#define RX_PIN 34			 // Audio from radio
#define TX_PIN DAC_CHANNEL_1 // AFSK audio output pin must use DAC_CHANNEL_1, not GPIO25
#define PTT_PIN 4			 // Push-to-Talk control pin
#ifndef LED_BUILTIN			 // LED to indicate PTT status
#define PTT_LED 2			 // Use GPIO2 if LED_BUILTIN is not defined
#else
#define PTT_LED LED_BUILTIN
#endif