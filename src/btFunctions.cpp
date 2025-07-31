#include <Arduino.h>
#include "btFunctions.h"
#include "afskEncode.h"
#include "configuration.h"


BluetoothSerial BTSerial; // Bluetooth KISS Interface

/**
 * @brief Initializes the Bluetooth serial interface with the specified device name.
 *
 * This function starts the Bluetooth serial communication using the device name defined by BT_NAME.
 * It also prints a message to the serial monitor indicating that the Bluetooth device is ready.
 */
void setupBluetooth()
{
  BTSerial.begin(BT_NAME); // Broadcast Bluetooth device name
  Serial.printf("%s %s\n", BT_NAME, "ready");
}

/**
 * @brief Checks if there is incoming data available on the Bluetooth serial interface.
 *
 * If data is available, reads up to 300 bytes from the Bluetooth serial buffer
 * and passes the received data to the transmitAX25 function for further processing.
 *
 * Call this function in loop() to handle incoming Bluetooth data.
 */
void checkBTforData()
{
  if (BTSerial.available())
  {
    uint8_t buf[300];
    size_t bytesRead = BTSerial.readBytes(buf, sizeof(buf));
    transmitAX25(buf, bytesRead);
  }
}
