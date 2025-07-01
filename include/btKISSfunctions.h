#ifndef BTKISSFUNCTIONS_H  // MUST be #ifndef (not #ifdef)
#define BTKISSFUNCTIONS_H

#include <Arduino.h>
#include <BluetoothSerial.h> // use the built-in BluetoothSerial library

extern BluetoothSerial BTSerial; // Bluetooth KISS Interface

void setupBluetooth(); // Function to initialize Bluetooth Serial communication
void sendKISSpacket(uint8_t *data, size_t len);	
void transmitAFSK(uint8_t *packet, size_t len);
void receiveAFSK();	

#endif // BTKISSFUNCTIONS_H