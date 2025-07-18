/**
 * @file configuration.h
 * @brief Configuration constants for ESP32 KISS TNC project.
 *
 * This header defines WiFi credentials, Bluetooth device name, network settings,
 * and pin assignments for the ESP32-based KISS TNC (Terminal Node Controller).
 *
 * Constants:
 * - BT_NAME: Bluetooth device name for identification.
 * - WIFI_SSID: WiFi network SSID.
 * - WIFI_PASSWORD: WiFi network password.
 * - LOCAL_IP: Static local IP address for the ESP32.
 * - GATEWAY: Network gateway IP address.
 * - SUBNET: Network subnet mask.
 *
 * Pin Definitions:
 * - PTT_PIN: GPIO pin used for Push-to-Talk (PTT) control.
 * - PTT_LED: GPIO pin connected to an LED indicating PTT status.
 * - TX_PIN: GPIO pin for transmitting audio to the radio.
 * - RX_PIN: GPIO pin for receiving audio from the radio.
 */
#include <Arduino.h> // for IPAddress

// Bluetooth device name for the KISS TNC
inline const char *BT_NAME = "ESP32 KISS TNC";

// WiFi configuration constants
inline const char *WIFI_SSID = "DCMNET";
inline const char *WIFI_PASSWORD = "0F1A2D3E4D5G6L7O8R9Y";
inline const IPAddress LOCAL_IP(192, 168, 0, 234);
inline const IPAddress GATEWAY(192, 168, 0, 1);
inline const IPAddress SUBNET(255, 255, 255, 0);

// Pin definitions for transceiver interface
inline const int PTT_PIN = 4; // Push-to-Talk control pin
inline const int PTT_LED = 2; // LED to indicate PTT status
inline const int TX_PIN = 25; // Audio to radio
inline const int RX_PIN = 34; // Audio from radio