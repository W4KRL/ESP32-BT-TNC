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

#include <Arduino.h>       // Include Arduino core for ESP32
#include "configuration.h" // for pin definitions
#include "waveTables.h"    // Include wave tables for AFSK modulation

/**
 * @brief Initializes the AFSK encoder hardware and loads necessary wave tables.
 *
 * Sets up the TX and PTT pins as outputs and ensures the PTT and PTT LED are off initially.
 * Attempts to load AFSK wave tables from non-volatile storage (NVS). If tables are not found,
 * generates and stores them in NVS, then halts execution to ensure tables are generated only once.
 * If tables are successfully loaded, continues normal operation.
 *
 * Serial output is used for status messages during initialization.
 */
void setupAFSKencoder()
{
  pinMode(TX_PIN, OUTPUT);    // Set TX pin as output
  pinMode(PTT_PIN, OUTPUT);   // Set PTT pin as output
  digitalWrite(PTT_PIN, LOW); // Ensure PTT is low initially (not transmitting)
  digitalWrite(PTT_LED, LOW); // Ensure PTT LED is off initially

  Serial.println("AFSK with Preferences Wave Tables");

  if (!loadTables())
  {
    Serial.println("Generating and storing tables in NVS...");
    generateAndStoreTables();
    Serial.println("Tables stored. Please restart device to load from NVS.");
    while (true)
    {
      delay(1000);
    } // halt to ensure single generation
  }
  else
  {
    Serial.println("Loaded wave tables successfully from NVS.");
  }
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
 * @brief Transmits a sequence of bits using AFSK (Audio Frequency-Shift Keying) modulation.
 *
 * This function encodes each bit in the input array as a tone using either the 1200 Hz or 2200 Hz sine wave table,
 * depending on the bit value. Each bit is transmitted by outputting the corresponding sine wave samples to the DAC pin.
 * After all bits are sent, the DAC output is set to its midpoint value.
 *
 * @param bits Pointer to the array of bits to transmit (each bit should be 0 or 1).
 * @param len Number of bits to transmit.
 *
 * @note The timing between samples is controlled by delayMicroseconds, based on the SAMPLE_RATE.
 *       Optionally, a small delay between bits can be added for timing stability.
 *       Ensure that sine1200, sine2200, TABLE_LEN, SAMPLE_RATE, TX_PIN, and dacWrite are defined elsewhere.
 */
void afskSend(uint8_t *bits, size_t len)
{
  for (size_t bit = 0; bit < len; bit++) {
    const uint8_t* table = bits[bit] ? sine1200 : sine2200;
    for (size_t i = 0; i < TABLE_LEN; i++) {
      dacWrite(TX_PIN, table[i]);
      delayMicroseconds(1000000 / SAMPLE_RATE);
    }
    // Optional: small delay between bits for timing stability
    // delay(20); // Remove or adjust as needed for your protocol
  }
  // Optionally, set DAC output to midpoint or zero after transmission
  dacWrite(TX_PIN, 128); // 128 is midpoint for 8-bit DAC
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
