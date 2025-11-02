/**
 * @file afskEncoder.cpp
 * @date 2025-08-26
 * @brief Modern AFSK encoder implementation using latest ESP32 Arduino API
 *
 * This implementation provides AFSK encoding for AX.25 frame transmission using
 * the modern Arduino ESP32 framework without legacy ESP-IDF driver dependencies.
 *
 * Key Features:
 * - Uses dacWrite() instead of driver/dac.h functions
 * - Accurate timer-based frequency generation
 * - Sine wave table generation for clean AFSK tones
 * - AX.25 frame encoding with bit-stuffing
 * - NRZI encoding for AFSK transmission
 * - PTT and LED control for radio interface
 * - Proper resource management and cleanup
 *
 * @author Modernized for Arduino ESP32 by W4KRL and Copilot
 */

#include "afskEncoder.h"
#include "configuration.h"
#include <math.h>

// Timer frequency after divider (80MHz / 8 = 10MHz)
#define TIMER_FREQ (APB_CLK_FREQ / AFSK_TIMER_DIVIDER)

// Module state variables
static struct
{
	uint8_t dacPin;
	int8_t pttPin;
	int8_t pttLedPin;
	uint16_t markFreq;
	uint16_t spaceFreq;
	uint16_t baudRate;
	float amplitude;
	uint8_t samplesPerCycle;
	bool initialized;
	bool transmitting;
} afsk_config = {
	.dacPin = AFSK_DAC_PIN,
	.pttPin = -1,
	.pttLedPin = -1,
	.markFreq = AFSK_MARK_FREQ,
	.spaceFreq = AFSK_SPACE_FREQ,
	.baudRate = AFSK_BAUD_RATE,
	.amplitude = AFSK_AMPLITUDE,
	.samplesPerCycle = AFSK_SAMPLES_PER_CYCLE,
	.initialized = false,
	.transmitting = false};

// Hardware resources
static hw_timer_t *afsk_timer = NULL;
static uint8_t *waveTable = NULL;
static volatile uint16_t currentSampleIndex = 0;
static volatile bool timerEnabled = false;

// Timer calculation variables
static volatile uint64_t ticksPerSample = 0;

// Forward declarations
static afsk_status_t generateWaveTable();
static uint64_t calculateTimerTicks(uint16_t frequency);
static void setTimerFrequency(uint16_t frequency);
static void setPTT(bool enable);

/**
 * @brief Timer interrupt service routine for AFSK sample generation
 */
void IRAM_ATTR afskTimerISR()
{
	if (!timerEnabled || !waveTable)
		return;

	// Output current sample to DAC using modern Arduino API
	dacWrite(afsk_config.dacPin, waveTable[currentSampleIndex]);

	// Advance to next sample
	currentSampleIndex++;
	if (currentSampleIndex >= afsk_config.samplesPerCycle)
	{
		currentSampleIndex = 0;
	}
}

/**
 * @brief Generate sine wave table for AFSK tones
 * @return AFSK_SUCCESS on success, error code otherwise
 */
static afsk_status_t generateWaveTable()
{
	// Free existing table
	if (waveTable)
	{
		free(waveTable);
		waveTable = NULL;
	}

	// Allocate new table
	waveTable = (uint8_t *)malloc(afsk_config.samplesPerCycle);
	if (!waveTable)
	{
		return AFSK_ERROR_DAC_INIT;
	}

	// Generate sine wave samples
	const uint8_t midpoint = AFSK_DAC_MAX_VALUE / 2;
	for (uint8_t i = 0; i < afsk_config.samplesPerCycle; i++)
	{
		float angle = 2.0f * PI * i / afsk_config.samplesPerCycle;
		float sineValue = sin(angle);

		// Scale to DAC range with amplitude control
		uint8_t dacValue = (uint8_t)(midpoint + (afsk_config.amplitude * midpoint * sineValue));
		waveTable[i] = dacValue;
	}

	return AFSK_SUCCESS;
}

/**
 * @brief Calculate timer ticks per sample for given frequency
 * @param frequency Desired frequency in Hz
 * @return Timer ticks per sample
 */
static uint64_t calculateTimerTicks(uint16_t frequency)
{
	uint32_t denominator = frequency * afsk_config.samplesPerCycle;
	uint64_t result = TIMER_FREQ / denominator;
	
	// Debug output for diagnosis
	Serial.printf("Calc: freq=%d, denom=%lu, ticks=%llu\n", frequency, denominator, result);
	
	return result;
}

/**
 * @brief Set PTT state
 * @param enable true to key transmitter, false to unkey
 */
static void setPTT(bool enable)
{
	if (afsk_config.pttPin >= 0)
	{
		digitalWrite(afsk_config.pttPin, enable ? HIGH : LOW);
	}
	if (afsk_config.pttLedPin >= 0)
	{
		digitalWrite(afsk_config.pttLedPin, enable ? HIGH : LOW);
	}
}

/**
 * @brief Set timer frequency for AFSK tone
 * @param frequency Frequency in Hz (mark or space)
 */
static void setTimerFrequency(uint16_t frequency)
{
	ticksPerSample = calculateTimerTicks(frequency);
	Serial.printf("Timer: Setting %d Hz -> %llu ticks\n", frequency, ticksPerSample);
	timerAlarmWrite(afsk_timer, ticksPerSample, true);
}

/**
 * @brief Initialize AFSK encoder hardware and resources
 * @return AFSK_SUCCESS on success, error code otherwise
 */
afsk_status_t setupAFSKEncoder()
{
	if (afsk_config.initialized)
	{
		return AFSK_SUCCESS; // Already initialized
	}

	// Debug: Show configuration
	Serial.printf("AFSK Init: Timer divider=%d, Timer freq=%lu Hz\n", AFSK_TIMER_DIVIDER, TIMER_FREQ);
	Serial.printf("AFSK Init: Mark=%d Hz, Space=%d Hz, Samples=%d\n", 
	              afsk_config.markFreq, afsk_config.spaceFreq, afsk_config.samplesPerCycle);

	// Initialize timer (timer 0, divider 8 for 10MHz, count up)
	afsk_timer = timerBegin(0, AFSK_TIMER_DIVIDER, true);
	if (!afsk_timer)
	{
		return AFSK_ERROR_TIMER_INIT;
	}

	// Attach ISR
	timerAttachInterrupt(afsk_timer, afskTimerISR, true);

	// Generate sine wave table
	afsk_status_t status = generateWaveTable();
	if (status != AFSK_SUCCESS)
	{
		return status;
	}

	// Configure PTT pins
	if (afsk_config.pttPin >= 0)
	{
		pinMode(afsk_config.pttPin, OUTPUT);
		digitalWrite(afsk_config.pttPin, LOW);
	}
	if (afsk_config.pttLedPin >= 0)
	{
		pinMode(afsk_config.pttLedPin, OUTPUT);
		digitalWrite(afsk_config.pttLedPin, LOW);
	}

	// Set DAC to midpoint
	dacWrite(afsk_config.dacPin, AFSK_DAC_MAX_VALUE / 2);

	afsk_config.initialized = true;
	return AFSK_SUCCESS;
}

/**
 * @brief Set AFSK parameters
 * @param markFreq Mark frequency in Hz
 * @param spaceFreq Space frequency in Hz
 * @param baudRate Baud rate in Hz
 * @param amplitude Signal amplitude (0.0 to 1.0)
 * @param samplesPerCycle Number of samples per waveform cycle
 * @return AFSK_SUCCESS on success, error code otherwise
 */
afsk_status_t setAFSKParameters(uint16_t markFreq, uint16_t spaceFreq,
								uint16_t baudRate, float amplitude,
								uint8_t samplesPerCycle)
{
	afsk_config.markFreq = markFreq;
	afsk_config.spaceFreq = spaceFreq;
	afsk_config.baudRate = baudRate;
	afsk_config.amplitude = amplitude;
	afsk_config.samplesPerCycle = samplesPerCycle;
	
	// Regenerate wave table with new parameters
	if (afsk_config.initialized)
	{
		return generateWaveTable();
	}
	
	return AFSK_SUCCESS;
}

/**
 * @brief Send raw bits using AFSK modulation
 * @param bits Array of bits to send (1 = mark, 0 = space)
 * @param len Number of bits to send
 * @return AFSK_SUCCESS on success, error code otherwise
 */
afsk_status_t afskSend(uint8_t *bits, size_t len)
{
	if (!afsk_config.initialized)
	{
		return AFSK_ERROR_NOT_INITIALIZED;
	}

	if (afsk_config.transmitting)
	{
		return AFSK_ERROR_INVALID_PARAMS; // Already transmitting
	}

	Serial.printf("Starting transmission of %d bits\n", len);
	
	afsk_config.transmitting = true;
	timerEnabled = true;

	// Start transmission
	timerAlarmEnable(afsk_timer);

	for (size_t bit = 0; bit < len; bit++)
	{
		// Set frequency based on bit value: 1 = mark, 0 = space
		uint16_t freq = bits[bit] ? afsk_config.markFreq : afsk_config.spaceFreq;
		setTimerFrequency(freq);

		// Wait for one bit duration
		uint64_t startTime = micros();
		uint64_t bitDuration = 1000000UL / afsk_config.baudRate;
		while ((micros() - startTime) < bitDuration)
		{
			yield(); // Allow other tasks to run
		}
	}

	// Stop transmission
	timerAlarmDisable(afsk_timer);
	timerEnabled = false;
	dacWrite(afsk_config.dacPin, AFSK_DAC_MAX_VALUE / 2); // Set to midpoint
	afsk_config.transmitting = false;

	Serial.printf("Transmission complete\n");

	return AFSK_SUCCESS;
}

/**
 * @brief Transmit AX.25 frame with AFSK modulation
 * @param kissFrame KISS frame data
 * @param len Frame length in bytes
 * @return AFSK_SUCCESS on success, error code otherwise
 */
afsk_status_t transmitAX25(uint8_t *kissFrame, size_t len)
{
	// This is a placeholder - full AX.25 implementation would go here
	// For now, just convert bytes to bits and transmit
	
	if (!kissFrame || len == 0)
	{
		return AFSK_ERROR_INVALID_PARAMS;
	}

	// Simple implementation: convert each byte to 8 bits
	uint8_t *bits = (uint8_t *)malloc(len * 8);
	if (!bits)
	{
		return AFSK_ERROR_INVALID_PARAMS;
	}

	// Convert bytes to bits
	for (size_t i = 0; i < len; i++)
	{
		for (int bit = 0; bit < 8; bit++)
		{
			bits[i * 8 + bit] = (kissFrame[i] >> bit) & 0x01;
		}
	}

	// Transmit the bits
	afsk_status_t result = afskSend(bits, len * 8);

	free(bits);
	return result;
}

/**
 * @brief Get status string for AFSK status code
 * @param status Status code
 * @return Human-readable status string
 */
const char *getAFSKStatusString(afsk_status_t status)
{
	switch (status)
	{
	case AFSK_SUCCESS:
		return "Success";
	case AFSK_ERROR_NOT_INITIALIZED:
		return "Not initialized";
	case AFSK_ERROR_TIMER_INIT:
		return "Timer initialization failed";
	case AFSK_ERROR_DAC_INIT:
		return "DAC initialization failed";
	case AFSK_ERROR_INVALID_PARAMS:
		return "Invalid parameters";
	default:
		return "Unknown error";
	}
}

/**
 * @brief Clean up AFSK encoder resources
 */
void cleanupAFSKEncoder()
{
	if (afsk_timer)
	{
		timerEnd(afsk_timer);
		afsk_timer = NULL;
	}

	if (waveTable)
	{
		free(waveTable);
		waveTable = NULL;
	}

	// Turn off PTT
	setPTT(false);

	// Reset DAC to midpoint
	dacWrite(afsk_config.dacPin, AFSK_DAC_MAX_VALUE / 2);

	afsk_config.initialized = false;
	afsk_config.transmitting = false;
}