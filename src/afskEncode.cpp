/**
 * @file afskEncode.cpp
 * @brief Implements AFSK (Audio Frequency-Shift Keying) encoding and AX.25 frame transmission for ESP32.
 *
 * This file provides functions to set up the AFSK encoder hardware, encode AX.25 frames with bit stuffing,
 * perform NRZI (Non-Return-to-Zero Inverted) encoding, and transmit the encoded data using AFSK modulation.
 * It is designed for use with Arduino/ESP32 environments and is suitable for amateur radio packet transmission.
 *
 * Functions:
 * - setupAFSKencoder(): Initializes GPIO pins and configures PWM for AFSK output.
 * - ax25Encode(): Performs AX.25 bit stuffing and frame flagging on input data.
 * - nrziEncode(): Applies NRZI encoding to the bit-stuffed data.
 * - afskSend(): Modulates the encoded bits as AFSK tones and transmits them.
 * - transmitAX25(): High-level function to transmit a KISS frame as an AX.25 packet using AFSK modulation.
 *
 * Dependencies:
 * - Arduino.h
 * - configuration.h (defines TX_PIN, RX_PIN, PTT_PIN, PTT_LED, etc.)
 *
 * Usage:
 * Call setupAFSKencoder() during initialization. Use transmitAX25() to send KISS frames.
 */
#include "afskEncode.h"

#include <Arduino.h>
#include "configuration.h"

void setupAFSKencoder()
{
  pinMode(TX_PIN, OUTPUT);    // Set TX pin as output
  pinMode(PTT_PIN, OUTPUT);   // Set PTT pin as output
  digitalWrite(PTT_PIN, LOW); // Ensure PTT is low initially (not transmitting)
  digitalWrite(PTT_LED, LOW); // Ensure PTT LED is off initially

  // Arduino-style LEDC setup for square wave output
  ledcSetup(0, 1200, 8);    // channel 0, 1200 Hz, 8-bit resolution
  ledcAttachPin(TX_PIN, 0); // attach TX_PIN to channel 0
  ledcWrite(0, 127);        // 50% duty cycle (127/255 for 8-bit)
}

size_t ax25Encode(uint8_t *input, size_t len, uint8_t *output)
{
  const uint8_t KISS_FRAME_FLAG = 0x7E; // KISS frame flag
  size_t outIndex = 0;
  output[outIndex++] = KISS_FRAME_FLAG;
  uint8_t buf = 0;
  int count = 0, ones = 0;
  for (size_t i = 0; i < len; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      bool bit = input[i] & (1 << j);
      if (bit)
        ones++;
      else
        ones = 0;
      buf |= bit << count++;
      if (ones == 5)
      {
        buf |= 0 << count++;
        ones = 0;
      }
      if (count == 8)
      {
        output[outIndex++] = buf;
        buf = 0;
        count = 0;
      }
    }
  }
  if (count)
    output[outIndex++] = buf;
  output[outIndex++] = KISS_FRAME_FLAG;
  return outIndex;
}

size_t nrziEncode(uint8_t *input, size_t len, uint8_t *output)
{
  bool last = true;
  size_t out = 0;
  for (size_t i = 0; i < len; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      bool bit = input[i] & (1 << j);
      if (!bit)
        last = !last;
      output[out++] = last;
    }
  }
  return out;
}

void afskSend(uint8_t *bits, size_t len)
{
  const unsigned long BIT_DURATION_US = 833;
  for (size_t i = 0; i < len; i++)
  {
    // Change frequency for each bit using Arduino-style API
    ledcSetup(0, bits[i] ? 1200 : 2200, 8); // channel 0, freq, 8-bit resolution
    ledcWrite(0, 127);                      // 50% duty cycle
    delayMicroseconds(BIT_DURATION_US);
  }
  ledcWrite(0, 0); // Turn off output after sending
}

void transmitAX25(uint8_t *kissFrame, size_t len)
{
  if (len == 0 || kissFrame[0] != 0x00)
  {
    return;
  }
  uint8_t *ax25 = kissFrame + 1;
  size_t ax25Len = len - 1;
  uint8_t stuffed[600], nrzi[600];
  size_t stuffedLen = ax25Encode(ax25, ax25Len, stuffed);
  size_t nrziLen = nrziEncode(stuffed, stuffedLen, nrzi);
  digitalWrite(PTT_PIN, HIGH);
  digitalWrite(PTT_LED, HIGH);
  delay(50);
  afskSend(nrzi, nrziLen);
  delay(50);
  digitalWrite(PTT_PIN, LOW);
  digitalWrite(PTT_LED, LOW);
}
