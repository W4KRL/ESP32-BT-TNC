#include <Arduino.h>
#include "btFunctions.h"
#include "afskFunctions.h"
#include "configuration.h"

BluetoothSerial BTSerial; // Bluetooth KISS Interface

void setupBluetooth()
{
  BTSerial.begin(BT_NAME); // Bluetooth name
  Serial.printf("%s %s", BT_NAME, "ready");
}

void checkBTforData()
{
  if (Serial.available())
  {
    uint8_t buf[300];
    size_t len = Serial.readBytes(buf, sizeof(buf));
    transmitAX25(buf, len);
  }

  if (BTSerial.available())
  {
    uint8_t buf[300];
    size_t len = BTSerial.readBytes(buf, sizeof(buf));
    transmitAX25(buf, len);
  }
}