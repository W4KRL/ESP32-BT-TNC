#include "goertzelFilter.h"

#include <Arduino.h>
#include "btFunctions.h" // Include Bluetooth functions
#include "configuration.h"

#define SAMPLE_RATE 9600  // Sample rate for AFSK demodulation
#define ADC_MIDPOINT 2048 // ESP32 12-bit resolution is 4096
#define GOERTZEL_N 64	  // Number of samples for Goertzel filter
#define MARK_FREQ 1200	  // Mark frequency for AFSK
#define SPACE_FREQ 2200	  // Space frequency for AFSK

// Coefficients for Goertzel filter
float coeffMark;
float coeffSpace;

/**
 * @brief Initializes the Goertzel filter coefficients for mark and space frequencies.
 *
 * This function calculates the angular frequencies for the MARK_FREQ and SPACE_FREQ
 * based on the defined SAMPLE_RATE. It then computes and assigns the corresponding
 * Goertzel filter coefficients (coeffMark and coeffSpace) using the cosine of the
 * calculated angular frequencies.
 *
 * Assumes that MARK_FREQ, SPACE_FREQ, SAMPLE_RATE, coeffMark, and coeffSpace are
 * defined and accessible in the current scope.
 */
void setupGoertzel()
{
	float omegaMark = 2.0 * PI * MARK_FREQ / SAMPLE_RATE;
	float omegaSpace = 2.0 * PI * SPACE_FREQ / SAMPLE_RATE;
	coeffMark = 2.0 * cos(omegaMark);
	coeffSpace = 2.0 * cos(omegaSpace);
	analogReadResolution(12); // Set ADC resolution to 12 bits for ESP32
}

/**
 * @brief Sends a data packet using the KISS protocol over Bluetooth serial.
 *
 * This function frames the provided data according to the KISS protocol,
 * escaping special characters as required, and writes the framed packet
 * to the BTSerial interface.
 *
 * @param data Pointer to the data buffer to be sent.
 * @param len  Length of the data buffer in bytes.
 *
 * The function automatically handles the insertion of KISS frame delimiters
 * and escapes any occurrence of the special characters FEND (0xC0) and FESC (0xDB)
 * within the data payload.
 */
void sendKISSpacket(uint8_t *data, size_t len)
{
	// KISS protocol special characters
	const uint8_t FEND = 0xC0;	// Frame End
	const uint8_t FESC = 0xDB;	// Frame Escape
	const uint8_t TFEND = 0xDC; // Transposed FEND
	const uint8_t TFESC = 0xDD; // Transposed FESC

	BTSerial.write(FEND);
	BTSerial.write(0x00); // Indicates start of KISS data frame

	for (size_t i = 0; i < len; i++)
	{
		if (data[i] == FEND)
		{
			BTSerial.write(FESC);
			BTSerial.write(TFEND);
		}
		else if (data[i] == FESC)
		{
			BTSerial.write(FESC);
			BTSerial.write(TFESC);
		}
		else
		{
			BTSerial.write(data[i]);
		}
	}
	BTSerial.write(FEND);
}

// CRC-16-CCITT implementation
/**
 * @brief Calculates the CRC-16-CCITT checksum for the given data.
 *
 * @param data Pointer to the data buffer.
 * @param len Length of the data buffer in bytes.
 * @return The computed 16-bit CRC value.
 */
uint16_t crc16_ccitt(uint8_t *data, size_t len)
{
	uint16_t crc = 0xFFFF;
	for (size_t i = 0; i < len; i++)
	{
		crc ^= (uint16_t)data[i] << 8;
		for (int j = 0; j < 8; j++)
			crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
	}
	return crc;
}

/**
 * @brief Processes a single decoded NRZ bit for AX.25/KISS frame detection and extraction.
 *
 * This function handles bit-stuffing removal, frame flag detection, and frame buffering.
 * When a complete frame is detected and its CRC is valid, the frame is passed to sendKISSpacket().
 *
 * @param bit The next NRZ bit to process (true for '1', false for '0').
 *
 * Internal logic:
 * - Removes bit-stuffing by counting consecutive '1's and skipping the 6th.
 * - Detects KISS frame flags using a shift register.
 * - Buffers frame bits into frameBuffer.
 * - On frame end, checks CRC (using crc16_ccitt) and sends valid frames.
 */
void handleBit(bool bit)
{
	const uint8_t KISS_FRAME_FLAG = 0x7E; // KISS frame flag
	bool lastNRZ = true;
	int oneCount = 0;
	uint32_t shiftReg = 0;
	bool inFrame = false;
	uint8_t frameBuffer[330];
	int frameIndex = 0;

	bool decoded = lastNRZ ^ bit;
	lastNRZ = bit;
	oneCount = decoded ? oneCount + 1 : 0;
	if (oneCount == 6)
	{
		return;
	}

	shiftReg = (shiftReg >> 1) | (decoded << 7);
	if (shiftReg == KISS_FRAME_FLAG)
	{
		if (inFrame && frameIndex >= 3)
		{
			uint16_t crc = crc16_ccitt(frameBuffer, frameIndex - 2);
			if (crc == 0xF0B8)
			{
				sendKISSpacket(frameBuffer, frameIndex - 2);
			}
		}
		frameIndex = 0;
		inFrame = true;
		return;
	}

	if (inFrame && frameIndex < sizeof(frameBuffer))
	{
		frameBuffer[frameIndex / 8] |= (decoded << (frameIndex % 8));
		frameIndex++;
	}
}

/**
 * @brief Processes a block of audio samples using the Goertzel algorithm to detect mark and space frequencies.
 *
 * This function reads a series of samples from the ADC, centers them around zero, and applies the Goertzel filter
 * to detect the presence of two specific frequencies (mark and space). It calculates the squared magnitude for each
 * frequency and determines which frequency is dominant in the current block of samples. The detected bit is then
 * passed to the handleBit function for further processing.
 *
 * Assumes the following external variables and constants are defined:
 * - GOERTZEL_N: Number of samples per block.
 * - RX_PIN: ADC pin to read samples from.
 * - ADC_MIDPOINT: Value to center the ADC reading around zero.
 * - coeffMark: Goertzel coefficient for the mark frequency.
 * - coeffSpace: Goertzel coefficient for the space frequency.
 * - handleBit(bool): Function to handle the detected bit.
 */
void processGoertzel()
{
	float qm1 = 0; // Previous sample for mark frequency
	float qm2 = 0; // Second previous sample for mark frequency
	float qs1 = 0; // Previous sample for space frequency
	float qs2 = 0; // Second previous sample for space frequency
	// Read a sample from the ADC, centered around 0
	// and process it through the Goertzel filter
	for (int i = 0; i < GOERTZEL_N; i++)
	{
		int sample = analogRead(RX_PIN) - ADC_MIDPOINT;
		float in = (float)sample;
		float new_qm = coeffMark * qm1 - qm2 + in;
		float new_qs = coeffSpace * qs1 - qs2 + in;
		qm2 = qm1;
		qm1 = new_qm;
		qs2 = qs1;
		qs1 = new_qs;
	}
	// Calculate the squared magnitude for each frequency using the Goertzel algorithm
	float magMark = qm1 * qm1 + qm2 * qm2 - qm1 * qm2 * coeffMark;
	float magSpace = qs1 * qs1 + qs2 * qs2 - qs1 * qs2 * coeffSpace;
	bool bit = magMark > magSpace;
	handleBit(bit);
}
