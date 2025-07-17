#ifndef BTFUNCTIONS_H
#define BTFUNCTIONS_H

#include <Arduino.h>
#include <BluetoothSerial.h> // use the built-in BluetoothSerial library

extern BluetoothSerial BTSerial; // Bluetooth KISS Interface

void setupBluetooth(); // Function to initialize Bluetooth Serial communication
void checkBTforData(); // Function to check Bluetooth Serial for incoming data

#endif // BTFUNCTIONS_H