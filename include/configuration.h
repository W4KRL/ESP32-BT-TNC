#include <Arduino.h> // for IPAddress

inline const char *BT_NAME = "ESP32 KISS TNC";

inline const char *WIFI_SSID = "DCMNET";
inline const char *WIFI_PASSWORD = "0F1A2D3E4D5G6L7O8R9Y";

/**
 * @brief Defines the local IP address for the device.
 *
 * This IP address is used to configure the network settings of the device.
 * Ensure that the IP address is within the subnet of your local network
 * and does not conflict with other devices on the network.
 *
 * Example: IPAddress(192, 168, 0, 234) represents the static IP address
 * 192.168.0.234.
 *
 * @note See instructions in Notes.md for setting up static IP address
 */
inline const IPAddress LOCAL_IP(192, 168, 0, 234);
inline const IPAddress GATEWAY(192, 168, 0, 1);
inline const IPAddress SUBNET(255, 255, 255, 0);

// Pin definitions for transceiver interface
inline const int PTT_PIN = 4; // Push-to-Talk control pin
inline const int PTT_LED = 2; // LED to indicate PTT status
inline const int TX_PIN = 25; // Audio to radio
inline const int RX_PIN = 34; // Audio from radio