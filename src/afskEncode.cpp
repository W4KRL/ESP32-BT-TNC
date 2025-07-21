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
 * - configuration.h (defines TX_PIN, PTT_PIN, PTT_LED, etc.)
 *
 * Usage:
 * Call setupAFSKencoder() during initialization. Use transmitAX25() to send KISS frames.
 */
#include "afskEncode.h"

#include <Arduino.h>
#include "configuration.h"

/**
 * @brief Initializes the AFSK encoder hardware.
 *
 * Configures the necessary pins and sets up the PWM (LEDC) for generating
 * a square wave output at 1200 Hz with 8-bit resolution on the TX pin.
 * Also ensures that the PTT (Push-To-Talk) pin and its associated LED
 * are set to their initial (inactive) states.
 *
 * Pins configured:
 * - TX_PIN: Output for AFSK signal (attached to LEDC channel 0)
 * - PTT_PIN: Output for controlling transmitter keying
 * - PTT_LED: Output for indicating PTT status
 */
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

/**
 * @brief Encodes input data into AX.25 format with bit-stuffing and KISS frame flags.
 *
 * This function takes a buffer of input bytes and encodes it according to the AX.25 protocol,
 * performing bit-stuffing (inserting a '0' after five consecutive '1's) and wrapping the output
 * with KISS frame flag bytes (0x7E) at the start and end. The encoded data is written to the
 * provided output buffer.
 *
 * @param input   Pointer to the input data buffer to be encoded.
 * @param len     Length of the input data buffer in bytes.
 * @param output  Pointer to the output buffer where encoded data will be stored.
 *                The buffer must be large enough to hold the encoded data.
 * @return        The number of bytes written to the output buffer.
 */
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

/**
 * @brief Encodes input data using NRZI (Non-Return-to-Zero Inverted) encoding.
 *
 * This function takes an array of bytes as input and encodes each bit using the NRZI scheme.
 * In NRZI encoding, a logical '0' causes a transition (inversion) in the output signal,
 * while a logical '1' causes no change. The encoded bits are written to the output array.
 *
 * @param input Pointer to the input byte array to be encoded.
 * @param len   Number of bytes in the input array.
 * @param output Pointer to the output array where encoded bits will be stored.
 *               Each bit is stored as a separate byte (0 or 1).
 * @return The number of encoded bits written to the output array.
 */
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

/**
 * @brief Sends a sequence of bits as AFSK (Audio Frequency-Shift Keying) tones.
 *
 * This function iterates over the provided bit array and transmits each bit
 * by switching between two frequencies (1200 Hz for '1', 2200 Hz for '0') using
 * the Arduino LEDC PWM API. Each bit is transmitted for a fixed duration.
 * After all bits are sent, the output is turned off.
 *
 * @param bits Pointer to the array of bits to send (each bit should be 0 or 1).
 * @param len Number of bits to send from the array.
 */
void afskSend(uint8_t *bits, size_t len)
{
  const unsigned long BIT_DURATION_US = 833;// Duration for each bit in microseconds (1200 Hz = 833 us, 2200 Hz = 455 us)
  for (size_t i = 0; i < len; i++)
  {
    // Change frequency for each bit using Arduino-style API
    ledcSetup(0, bits[i] ? 1200 : 2200, 8); // channel 0, freq, 8-bit resolution
    ledcWrite(0, 127);                      // 50% duty cycle
    delayMicroseconds(BIT_DURATION_US);
  }
  ledcWrite(0, 0); // Turn off output after sending
}

/**
 * @brief Transmits an AX.25 frame using AFSK modulation.
 *
 * This function takes a KISS frame, performs AX.25 encoding, bit stuffing,
 * and NRZI encoding, then transmits the resulting data using AFSK modulation.
 * It also handles PTT (Push-To-Talk) control and LED indication.
 *
 * @param kissFrame Pointer to the input KISS frame (first byte should be 0x00).
 * @param len Length of the KISS frame in bytes.
 */
void transmitAX25(uint8_t *kissFrame, size_t len)
{
  if (len == 0 || kissFrame[0] != 0x00) // Quit if no data or first byte is not 0x00
  {
    return;
  }
  uint8_t *ax25 = kissFrame + 1;                          // Skip the first byte (0x00)
  size_t ax25Len = len - 1;                               // Length of AX.25 data (excluding the first byte)
  uint8_t stuffed[600], nrzi[600];                        // Buffers for stuffed and NRZI encoded data
  size_t stuffedLen = ax25Encode(ax25, ax25Len, stuffed); // Perform AX.25 encoding with bit stuffing
  size_t nrziLen = nrziEncode(stuffed, stuffedLen, nrzi); // Perform NRZI encoding
  digitalWrite(PTT_PIN, HIGH);                            // Key the transmitter
  digitalWrite(PTT_LED, HIGH);                            // Turn on PTT LED
  delay(50);                                              // Allow time for PTT to engage
  afskSend(nrzi, nrziLen);                                // Send the NRZI encoded bits as AFSK tones
  delay(50);                                              // Allow time for transmission to complete
  digitalWrite(PTT_PIN, LOW);                             // Unkey the transmitter
  digitalWrite(PTT_LED, LOW);                             // Turn off PTT LED
}
