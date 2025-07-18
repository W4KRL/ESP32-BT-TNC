#ifndef GOERTZELFILTER_H
#define GOERTZELFILTER_H	

#include <Arduino.h>

void setupGoertzel();   // Call in setup() to initialize Goertzel filter coefficients
void processGoertzel(); // Call in loop() to process a single sample through the Goertzel filter

#endif // GOERTZELFILTER_H
