#ifndef GOERTZELFILTER_H
#define GOERTZELFILTER_H	

#include <Arduino.h>

void setupGoertzel(); // Function to initialize Goertzel filter coefficients
uint16_t crc16_ccitt(uint8_t* data, size_t len); // Function to calculate CRC16-CCITT
void processGoertzel(); // Function to process a single sample through the Goertzel filter

#endif // GOERTZELFILTER_H
