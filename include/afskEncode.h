/**
 * @file afskEncode.h
 * @date 2025-07-31
 * @brief AFSK (Audio Frequency-Shift Keying) encoder interface for AX.25 frame transmission on Arduino-based platforms.
 *
 * This header provides function declarations for configuring and using an AFSK encoder for transmitting AX.25 protocol frames.
 *
 * Functions:
 * - setupAFSKencoder(): Initializes and configures the AFSK modulation settings. Should be called in the Arduino setup() function.
 * - transmitAX25(uint8_t *kissFrame, size_t len): Transmits an AX.25 frame using AFSK modulation. Intended to be called when AX.25 data is ready to send.
 */

#ifndef AFSK_ENCODE_H
#define AFSK_ENCODE_H

#include <Arduino.h>

void setupAFSKencoder();
void transmitAX25(uint8_t *kissFrame, size_t len);

#endif // AFSK_ENCODE_H
