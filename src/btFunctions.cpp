#include <Arduino.h>
#include "btFunctions.h"
#include "afskFunctions.h"
#include "configuration.h"

BluetoothSerial BTSerial; // Bluetooth KISS Interface

void setupBluetooth()
{
  BTSerial.begin(BT_NAME); // Broadcast Bluetooth device name
  Serial.printf("%s %s\n", BT_NAME, "ready");
}

void checkBTforData()
{
  if (BTSerial.available())
  {
    uint8_t buf[300];
    size_t bytesRead = BTSerial.readBytes(buf, sizeof(buf));
    transmitAX25(buf, bytesRead);
  }
}