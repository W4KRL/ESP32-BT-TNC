#ifndef AFSKFUNCTIONS_H
#define AFSKFUNCTIONS_H

#include <Arduino.h>

void setupAFSK();								   // Call in setup() to configure AFSK modulation settings
void transmitAX25(uint8_t *kissFrame, size_t len); // Call in loop() to transmit AX.25 frames using AFSK modulation

#endif // AFSKFUNCTIONS_H
