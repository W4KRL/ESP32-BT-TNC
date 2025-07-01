#ifndef KISSFUNCTIONS_H
#define KISSFUNCTIONS_H

#include <Arduino.h>
#include "btFunctions.h" // Include Bluetooth KISS functions

void sendKISSpacket(uint8_t *data, size_t len); // Function to send KISS packet over Bluetooth Serial

#endif // KISSFUNCTIONS_H
