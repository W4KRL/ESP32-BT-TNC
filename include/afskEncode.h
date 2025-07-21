#ifndef AFSK_ENCODE_H
#define AFSK_ENCODE_H

#include <Arduino.h>

void setupAFSKencoder();								   // Call in setup() to configure AFSK modulation settings
void transmitAX25(uint8_t *kissFrame, size_t len); // Call in loop() to transmit AX.25 frames using AFSK modulation

#endif // AFSK_ENCODE_H
