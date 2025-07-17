
/**
 * @file wifiConnection.cpp
 * @brief Implements Wi-Fi connection and OTA update functionality for the magloop-controller project.
 *
 * This file provides functions to initialize Wi-Fi connectivity, configure static IP,
 * handle built-in LED status indication, and enable Arduino OTA (Over-The-Air) updates.
 * It uses the ESP32 WiFi and ArduinoOTA libraries, and relies on configuration parameters
 * defined in "configuration.h" for SSID, password, and network settings.
 *
 * Features:
 * - Wi-Fi connection with status LED feedback.
 * - Static IP configuration.
 * - Built-in LED control and toggling.
 * - Arduino OTA update support with progress and error reporting.
 *
 * Dependencies:
 * - Arduino.h
 * - WiFi.h
 * - ArduinoOTA.h
 * - configuration.h
 *
 * @author Karl Berger
 * @date 2025-05-20
 */
#include "wifiConnection.h" // Wi-Fi connection header

#include <Arduino.h>	   // for PlatformIO
#include <WiFi.h>		   // for WiFi
#include <ArduinoOTA.h>	   // for OTA updates
#include "configuration.h" // for SSID, password, static IP

/**
 * @brief Initializes and configures Over-The-Air (OTA) update functionality.
 *
 * This function sets up the ArduinoOTA library, registering event handlers for OTA start,
 * end, progress, and error events. It enables the device to receive firmware or filesystem
 * updates wirelessly. Upon successful initialization, a message is printed to the serial console.
 *
 * Event handlers:
 * - onStart: Prints the type of update being started ("sketch" or "filesystem").
 * - onEnd: Prints a message when the update is complete.
 * - onProgress: Prints the update progress as a percentage.
 * - onError: Prints an error message corresponding to the OTA error encountered.
 */
void otaBegin()
{
	ArduinoOTA.begin();
	ArduinoOTA.onStart([]()
					   {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.println("Start updating " + type); });
	ArduinoOTA.onEnd([]()
					 { Serial.println("\nUpdate Complete"); });
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
						  { Serial.printf("Progress: %u%%\r", (progress * 100) / total); });
	ArduinoOTA.onError([](ota_error_t error)
					   {
					   Serial.printf("Error[%u]: ", error);
					   if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
					   else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
					   else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
					   else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
					   else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
	Serial.println("OTA Ready");
} // otaBegin()

//! Global variables
static bool ledBuiltIn = LOW; // Built-in LED LOW = OFF, HIGH = ON

/**
 * @brief Sets the state of the built-in LED.
 *
 * This function updates the state of the built-in LED to the specified value.
 * It also updates the internal `ledBuiltIn` variable to reflect the current state.
 *
 * @param state A boolean value indicating the desired LED state:
 *              - `true` to turn the LED ON.
 *              - `false` to turn the LED OFF.
 */
void setLED_BUILTIN(bool state)
{
	ledBuiltIn = state;
	digitalWrite(LED_BUILTIN, ledBuiltIn);
}

/**
 * @brief Toggles the state of the built-in LED.
 *
 * This function inverts the current state of the built-in LED. If the LED is ON,
 * it will be turned OFF, and vice versa. The internal `ledState` variable is
 * updated to reflect the new state.
 */
void toggleLED_BUILTIN()
{
	ledBuiltIn = !ledBuiltIn;			   // Invert the current state
	digitalWrite(LED_BUILTIN, ledBuiltIn); // Apply the new state
}

/**
 * @brief Attempts to connect to a WiFi network using predefined SSID and password.
 *
 * If the device is not already connected to WiFi, this function initiates the connection
 * process. While attempting to connect, it toggles the built-in LED and prints a dot to
 * the serial monitor every 250 milliseconds. Once connected, it turns on the built-in LED
 * and prints the assigned local IP address to the serial monitor.
 *
 * @note Requires WIFI_SSID and WIFI_PASSWORD to be defined.
 * @note Assumes Serial and WiFi have been initialized.
 */
void wifiConnect()
{
	if (!WiFi.isConnected())
	{
		WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
		while (WiFi.status() != WL_CONNECTED)
		{
			toggleLED_BUILTIN();
			delay(250);
			Serial.print(".");
		}
		setLED_BUILTIN(HIGH); // Turn on the LED when connected
		Serial.printf("\n%s: %s\n", "Connected to IP Address", WiFi.localIP().toString());
	}
} // wifiConnect()

/**
 * @brief Initializes the WiFi connection with specific settings.
 *
 * This function configures the onboard LED as an output and turns it off.
 * It sets the WiFi mode to station (WIFI_STA), disables WiFi persistence,
 * enables automatic reconnection, and disables WiFi sleep mode.
 * It then attempts to configure the WiFi with a static IP address using
 * the predefined LOCAL_IP, GATEWAY, and SUBNET values.
 * If static IP configuration fails, an error message is printed to Serial.
 */
void wifiBegin()
{
	// Configure onboard LED pin
	pinMode(LED_BUILTIN, OUTPUT);	// Set LED_BUILTIN as output
	digitalWrite(LED_BUILTIN, LOW); // Start with LED off

	WiFi.mode(WIFI_STA);
	WiFi.persistent(false);
	WiFi.setAutoReconnect(true);
	WiFi.setSleep(false);
	Serial.printf("\n%s %s\n", "Connecting to", WIFI_SSID);
	// Set static IP configuration
	if (!WiFi.config(LOCAL_IP, GATEWAY, SUBNET))
	{
		Serial.println("Static IP Configuration Failed!");
		return;
	}
} // wifiBegin()