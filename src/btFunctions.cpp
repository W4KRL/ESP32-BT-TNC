#include <Arduino.h>
#include "btFunctions.h"
#include "afskFunctions.h"

BluetoothSerial BTSerial; // Bluetooth KISS Interface

void setupBluetooth()
{
  BTSerial.begin("ESP32_KISS_TNC"); // Bluetooth name
  Serial.println("ESP32 KISS TNC Ready");
}	


