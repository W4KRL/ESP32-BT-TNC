#ifndef BTKISSFUNCTIONS_H
#define BTKISSFUNCTIONS_H

#include <Arduino.h>
#include <BluetoothSerial.h> // use the built-in BluetoothSerial library

extern BluetoothSerial BTSerial; // Bluetooth KISS Interface

void setupBluetooth();							// Function to initialize Bluetooth Serial communication
void sendKISSpacket(uint8_t *data, size_t len); // Function to send KISS packet over Bluetooth Serial
void transmitAFSK(uint8_t *packet, size_t len); // Function to transmit AFSK modulated data
void receiveAFSK();								// Function to receive AFSK modulated data

#endif // BTKISSFUNCTIONS_H