#ifndef KISSFUNCTIONS_H
#define KISSFUNCTIONS_H

#include <Arduino.h>
#include "btFunctions.h" // Include Bluetooth KISS functions

void sendKISSpacket(uint8_t *data, size_t len); // Function to send KISS packet over Bluetooth Serial
size_t ax25Encode(uint8_t *input, size_t len, uint8_t *output); // Function to encode AX.25 frames

extern const uint8_t KISS_FRAME_FLAG; // KISS frame flag	

#endif // KISSFUNCTIONS_H
