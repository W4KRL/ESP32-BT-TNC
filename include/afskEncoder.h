/**
 * @file afskEncoder.h
 * @date 2025-08-26
 * @brief Modern AFSK (Audio Frequency-Shift Keying) encoder using latest ESP32 Arduino DAC API
 *
 * This header provides function declarations for configuring and using an AFSK encoder
 * for transmitting AX.25 protocol frames using the modern Arduino ESP32 framework.
 *
 * Key improvements over legacy version:
 * - Uses modern Arduino ESP32 DAC API (dacWrite) instead of driver/dac.h
 * - Improved timer frequency calculations for accurate AFSK generation
 * - Better resource management and error handling
 * - Configurable parameters for different AFSK configurations
 *
 * Hardware Requirements:
 * - ESP32 with DAC capability (GPIO25 or GPIO26)
 * - PTT control pin for transmitter keying
 * - Optional PTT LED indicator
 *
 * Usage:
 * 1. Call setupAFSKEncoder() during Arduino setup() to initialize hardware
 * 2. Use transmitAX25() to send KISS frames as AX.25 packets via AFSK
 * 3. Use afskSend() for raw bit transmission (testing purposes)
 * 4. Call cleanupAFSKEncoder() when done to free resources
 *
 * @author Original AFSK encoding by 2E0UMR
 * @author Modernized for Arduino ESP32 by W4KRL
 * @see https://uhpowerup.com/
 */

#ifndef AFSK_ENCODER_H
#define AFSK_ENCODER_H

#include <Arduino.h>

// Configuration constants
#define AFSK_DAC_PIN 25			  // GPIO25 (DAC1) - can be changed to 26 (DAC2)
#define AFSK_MARK_FREQ 1200		  // Mark frequency (Hz) - logical 1
#define AFSK_SPACE_FREQ 2200	  // Space frequency (Hz) - logical 0
#define AFSK_BAUD_RATE 1200		  // Baud rate (bits per second)
#define AFSK_SAMPLES_PER_CYCLE 32 // Samples per waveform cycle (power of 2)
#define AFSK_AMPLITUDE 0.8f		  // Amplitude (0.0 to 1.0)
#define AFSK_DAC_MAX_VALUE 255	  // 8-bit DAC maximum value
#define AFSK_TIMER_DIVIDER 8	  // 80MHz / 8 = 10MHz timer frequency for finer control

// Error codes
typedef enum
{
	AFSK_SUCCESS = 0,
	AFSK_ERROR_INVALID_PIN,
	AFSK_ERROR_TIMER_INIT,
	AFSK_ERROR_DAC_INIT,
	AFSK_ERROR_INVALID_PARAMS,
	AFSK_ERROR_NOT_INITIALIZED,
	AFSK_ERROR_BUFFER_OVERFLOW
} afsk_status_t;

/**
 * @brief Initialize the AFSK encoder with default settings
 * @param dacPin DAC output pin (25 or 26)
 * @param pttPin PTT control pin (-1 to disable)
 * @param pttLedPin PTT LED indicator pin (-1 to disable)
 * @return AFSK_SUCCESS on success, error code otherwise
 */
afsk_status_t setupAFSKEncoder(uint8_t dacPin, int8_t pttPin, int8_t pttLedPin);

/**
 * @brief Initialize the AFSK encoder with default pins from configuration.h
 * @return AFSK_SUCCESS on success, error code otherwise
 */
afsk_status_t setupAFSKEncoder();

/**
 * @brief Configure AFSK parameters
 * @param markFreq Mark frequency in Hz
 * @param spaceFreq Space frequency in Hz
 * @param baudRate Baud rate in bits per second
 * @param amplitude Amplitude (0.0 to 1.0)
 * @param samplesPerCycle Samples per waveform cycle (power of 2)
 * @return AFSK_SUCCESS on success, error code otherwise
 */
afsk_status_t setAFSKParameters(uint16_t markFreq, uint16_t spaceFreq,
								uint16_t baudRate, float amplitude,
								uint8_t samplesPerCycle);

/**
 * @brief Transmit an AX.25 frame using AFSK modulation
 * @param kissFrame Pointer to KISS frame (first byte should be 0x00)
 * @param len Length of KISS frame in bytes
 * @return AFSK_SUCCESS on success, error code otherwise
 */
afsk_status_t transmitAX25(uint8_t *kissFrame, size_t len);

/**
 * @brief Transmit raw bits using AFSK modulation (for testing)
 * @param bits Pointer to array of bits (each byte should be 0 or 1)
 * @param len Number of bits to transmit
 * @return AFSK_SUCCESS on success, error code otherwise
 */
afsk_status_t afskSend(uint8_t *bits, size_t len);

/**
 * @brief Check if AFSK encoder is currently transmitting
 * @return true if transmitting, false otherwise
 */
bool isAFSKTransmitting();

/**
 * @brief Get string description of status code
 * @param status Status code to convert
 * @return String description
 */
const char *getAFSKStatusString(afsk_status_t status);

/**
 * @brief Clean up AFSK encoder resources
 */
void cleanupAFSKEncoder();

// Internal functions (not for external use)
size_t ax25Encode(uint8_t *input, size_t len, uint8_t *output);
size_t nrziEncode(uint8_t *input, size_t len, uint8_t *output);

#endif // AFSK_ENCODER_H
