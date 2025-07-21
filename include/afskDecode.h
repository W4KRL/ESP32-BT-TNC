#ifndef AFSK_DECODE_H
#define AFSK_DECODE_H	

#include <Arduino.h>

void setupAFSKdecoder();   // Call in setup() to initialize Goertzel filter coefficients
void receiveAFSK(); // Call in loop() to process a single sample through the Goertzel filter

#endif // AFSK_DECODE_H
