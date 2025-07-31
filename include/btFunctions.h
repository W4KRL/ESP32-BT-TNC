/**
 * @file btFunctions.h
 * @date 2025-07-31
 * @brief Bluetooth Serial interface functions for ESP32 KISS TNC project.
 *
 * This header declares functions and variables for initializing and handling
 * Bluetooth Serial communication using the built-in BluetoothSerial library.
 *
 * - setupBluetooth(): Initializes Bluetooth Serial communication. Call in setup().
 * - checkBTforData(): Checks for incoming Bluetooth Serial data. Call in loop().
 *
 * @note Externally declares BTSerial as the Bluetooth KISS interface.
 */
#ifndef BTFUNCTIONS_H
#define BTFUNCTIONS_H

#include <Arduino.h>
#include <BluetoothSerial.h> // use the built-in BluetoothSerial library

extern BluetoothSerial BTSerial; // Bluetooth KISS Interface

void setupBluetooth(); // Call in setup() to initialize Bluetooth Serial communication
void checkBTforData(); // Call in loop() to check Bluetooth Serial for incoming data

#endif // BTFUNCTIONS_H
