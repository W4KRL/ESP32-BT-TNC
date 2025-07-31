/**
 * @file afskDecode.h
 * @date 2025-07-31
 * @brief Header file for AFSK (Audio Frequency-Shift Keying) decoder functions.
 *
 * This file declares functions for initializing and processing AFSK decoding using the Goertzel algorithm.
 *
 * Functions:
 * - setupAFSKdecoder(): Initializes the Goertzel filter coefficients. Should be called in the setup() function.
 * - receiveAFSK(): Processes a single audio sample through the Goertzel filter. Should be called in the loop() function.
 */
#ifndef AFSK_DECODE_H
#define AFSK_DECODE_H

#include <Arduino.h>

void setupAFSKdecoder(); // Call in setup() to initialize Goertzel filter coefficients
void receiveAFSK();		 // Call in loop() to process a single sample through the Goertzel filter

#endif // AFSK_DECODE_H
