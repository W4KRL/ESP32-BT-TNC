#ifndef AFSKFUNCTIONS_H
#define AFSKFUNCTIONS_H

#include <Arduino.h>

void setupAFSK();								// Function to set up AFSK modulation settings
void transmitAFSK(uint8_t *packet, size_t len); // Function to transmit AFSK modulated data
void receiveAFSK();								// Function to receive AFSK modulated data

#endif // AFSKFUNCTIONS_H