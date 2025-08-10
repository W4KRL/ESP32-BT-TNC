/**
 * @file wifiConnection.h
 * @brief Declarations for WiFi and OTA connection management functions.
 *
 * This header provides function declarations for initializing WiFi,
 * connecting to a WiFi network, and starting OTA (Over-The-Air) updates.
 * 
 * @author Karl Berger
 * @date 2025-05-20
 */

/**
 * @brief Initializes WiFi hardware and configuration.
 *
 * Prepares the device for WiFi operations. Should be called before attempting to connect.
 */
void wifiBegin();

/**
 * @brief Connects the device to the configured WiFi network.
 *
 * Attempts to establish a connection using the current WiFi settings.
 * Called in loop() to restore a connection if lost.
 */
void wifiConnect();

/**
 * @brief Initializes OTA (Over-The-Air) update functionality.
 *
 * Sets up the device to receive firmware updates over the network.
 */
void otaBegin();