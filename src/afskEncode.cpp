/**
 * @file afskEncode.cpp
 * @date
 * @brief Implements AFSK (Audio Frequency-Shift Keying) encoding and transmission for AX.25 packet radio on ESP32.
 *
 * This file provides functions to generate sine waveforms for AFSK modulation, perform AX.25 frame encoding with bit-stuffing,
 * apply NRZI (Non-Return-to-Zero Inverted) encoding, and transmit the resulting data using the ESP32 DAC hardware.
 * It also manages PTT (Push-To-Talk) control and LED indication for radio transmission.
 *
 * Key Features:
 * - Sine wave table generation for configurable amplitude and frequency.
 * - Timer-based DAC output for precise waveform generation.
 * - AX.25 frame encoding with bit-stuffing and KISS frame flag insertion.
 * - NRZI encoding for bitstream preparation.
 * - AFSK transmission at 1200/2200 Hz for AX.25 packet radio.
 * - PTT and LED control for transmitter keying.
 *
 * Dependencies:
 * - Arduino core for ESP32
 * - ESP32 DAC driver
 * - Pin definitions from configuration.h
 *
 * Usage:
 * 1. Call setupAFSKencoder() during initialization to configure hardware and generate wave tables.
 * 2. Use transmitAX25() to send a KISS frame as an AX.25 packet via AFSK modulation.
 *
 * @author AFSK encoding by 2E0UMR
 * @see https://uhpowerup.com/
 * @author adapted to PlatformIO, added sinewave generation by W4KRL

 */
#include "afskEncode.h"

#include <Arduino.h>       // Include Arduino core for ESP32
#include "configuration.h" // for pin definitions
#include "driver/dac.h"    // Include DAC driver for audio output

#define DAC_CHANNEL DAC_CHANNEL_1 // the waveform output pin. (e.g., DAC_CHANNEL_1 (25) or DAC_CHANNEL_2 (26))

// Constants for the ESP32 timer and DAC
// The ESP32 timer runs at 80 MHz, so we need to divide it down for our use
// The Arduino library includes a macro for the APB clock frequency
#define TIMER_DIVIDER 8                                         // for 0.1 uS resolution divide ESP32 80 MHz timer by 8
const uint64_t TICKS_PER_SECOND = APB_CLK_FREQ / TIMER_DIVIDER; // set the timer resolution to 0.1 uS per tick (10 MHz)
#define MAX_DAC_VALUE 255                                       // the maximum ESP32 DAC value
uint64_t ticksPerSample = 0;                                    // timer ticks per cycle (set at run time)
hw_timer_t *timer = NULL;                                       // instantiate the timer

// Configurable items
const unsigned int SAMPLES_PER_CYCLE = 32; // the number of samples per cycle of the waveform power of 2 (e.g., 32, 64, 128, etc.
float frequency = 1200;                    // the desired frequency (Hz) of the output waveform
float amplitude = 1.0;                     // the amplitude of the waveform (0.0 to 1.0, where 1.0 is the maximum DAC value)

// the global array that will hold all of the digital waveform values
unsigned int waveTable[SAMPLES_PER_CYCLE];

/**
 * The function populates the waveValues array with one complete cycle of sinusoid data.
 * It calculates the value for each sample step based on the desired frequency and amplitude.
 * The values are calculated using the sine function and scaled to fit within the DAC range.
 * @param samplesPerCycle The number of samples per cycle of the waveform.
 * @note The samplesPerCycle should be a power of 2 (e.g., 32, 64, 128, etc.).
 * @param amplitude The amplitude of the sine wave, scaled to fit within the DAC range.
 * @note The amplitude should be between 0.0 and 1.0, where 1.0 represents the maximum DAC value.
 */
void populateWaveTable(float amplitude)
{
  for (int i = 0; i < SAMPLES_PER_CYCLE; i++)
  {
    float angleInRadians = float(i) * 2.0 * PI / float(SAMPLES_PER_CYCLE);
    waveTable[i] = round((MAX_DAC_VALUE / 2.0) + (amplitude * (MAX_DAC_VALUE / 2.0) * sin(angleInRadians)));
    // Serial.printf("step: %d, radians: %.6f, value: %d\n", i, angleInRadians, value);
  }
}

/**
 * @brief Timer interrupt service routine for waveform output.
 *
 * This function is marked with IRAM_ATTR to ensure it is placed in IRAM for fast execution
 * as it is called from a hardware timer interrupt. It outputs the next value
 * from a precomputed waveform table to the DAC, cycling through the table to generate a
 * continuous waveform. The function maintains a static index to track the current position
 * in the waveform table and wraps around when the end of the table is reached.
 *
 * MARK and SPACE waveforms are nearly phase continuous due to no reset of currentSampleIdx.
 *
 * Note: All operations and function calls within this ISR must be IRAM-safe.
 */
void IRAM_ATTR onTimer()
{
  static unsigned currentSampleIdx = 0;                    // static variable to keep track of the current sample index
  unsigned int sampleValue = waveTable[currentSampleIdx];  // get the waveform value from the array
  dac_output_voltage(DAC_CHANNEL, sampleValue);            // output the voltage to the DAC_CHANNEL (IRAM-safe)
  currentSampleIdx++;                                      // advance the array index
  currentSampleIdx = currentSampleIdx % SAMPLES_PER_CYCLE; // reset the index if it exceeds the array size
}

/**
 * @brief Sets up the timer to call the onTimer() function at the desired sample rate
 *
 * The timer will call the onTimer() function at the precise rate needed to produce
 * the desired output frequency.
 */
void setupCallbackTimer(uint64_t ticks_per_sample)
{
  // The rate at which the timer will call the onTimer() function
  // is ticks_per_sample times the timer resolution (0.1 uS)
  // Timer resolution is TIMER_DIVIDER / APB_CLK_FREQ

  timerAlarmWrite(timer, ticks_per_sample, true); // autoreload = true
  timerAlarmEnable(timer);
}

/**
 * @brief Initializes the AFSK encoder and sets up the DAC output.
 *
 * This function configures the PTT pin, PTT LED pin, populates the waveform table,
 * and enables the DAC output. It ensures that the PTT is initially low (not transmitting)
 * and sets the DAC output to a midpoint value.
 */
void setupAFSKencoder()
{
  pinMode(PTT_PIN, OUTPUT);                   // Set PTT pin as output
  digitalWrite(PTT_PIN, LOW);                 // Ensure PTT is low initially (not transmitting)
  pinMode(PTT_LED, OUTPUT);                   // Set PTT LED pin as output
  digitalWrite(PTT_LED, LOW);                 // Ensure PTT LED is off initially
  populateWaveTable(1.0);                     // Populate the waveform values for AFSK modulation
  dac_output_enable(DAC_CHANNEL);             // Enable DAC output on the specified channel
  dac_output_voltage(DAC_CHANNEL, 128);       // Set DAC output to midpoint (half the DAC range, not true 0V)
  timer = timerBegin(0, TIMER_DIVIDER, true); // Initialize the timer with ID 0, divider, and count up
  timerAttachInterrupt(timer, onTimer, true); // Attach the timer interrupt handler
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
  const uint64_t TICKS_PER_SAMPLE1200 = TICKS_PER_SECOND / (1200 * SAMPLES_PER_CYCLE); // Ticks per sample for 1200 Hz
  const uint64_t TICKS_PER_SAMPLE2200 = TICKS_PER_SECOND / (2200 * SAMPLES_PER_CYCLE); // Ticks per sample for 2200 Hz

  timerAlarmEnable(timer); // Enable the timer to start generating the waveform
  for (size_t bit = 0; bit < len; bit++)
  {
    // If the current bit is '1', use 1200 Hz (sine1200Ticks); if '0', use 2200 Hz (sine2200Ticks)
    ticksPerSample = (bits[bit] ? TICKS_PER_SAMPLE1200 : TICKS_PER_SAMPLE2200);
    timerAlarmWrite(timer, ticksPerSample, true);

    uint64_t start = micros(); // Get the current time in microseconds
    while ((micros() - start) < (1000000 / 1200))
    {
      yield(); // Yield to allow other tasks to run (optional)
    }
  }
  timerAlarmDisable(timer);                           // Disable the timer after sending all bits
  dac_output_voltage(DAC_CHANNEL, MAX_DAC_VALUE / 2); // Set DAC output to midpoint (half the DAC range)
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
