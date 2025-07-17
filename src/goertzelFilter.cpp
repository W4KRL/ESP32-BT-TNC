#include "goertzelFilter.h"

#include <Arduino.h>
#include "kissFunctions.h"
#include "configuration.h"

#define SAMPLE_RATE 9600
#define GOERTZEL_N 64
#define MARK_FREQ 1200
#define SPACE_FREQ 2200

// Coefficients for Goertzel filter
float coeffMark;
float coeffSpace;

void setupGoertzel()
{
	float omegaMark = 2.0 * PI * MARK_FREQ / SAMPLE_RATE;
	float omegaSpace = 2.0 * PI * SPACE_FREQ / SAMPLE_RATE;
	coeffMark = 2.0 * cos(omegaMark);
	coeffSpace = 2.0 * cos(omegaSpace);
}

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

void handleBit(bool bit)
{
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

void processGoertzel()
{
	float qm1 = 0;
	float qm2 = 0;
	float qs1 = 0;
	float qs2 = 0;
	for (int i = 0; i < GOERTZEL_N; i++)
	{
		int sample = analogRead(RX_PIN) - 2048;
		float in = (float)sample;
		float new_qm = coeffMark * qm1 - qm2 + in;
		float new_qs = coeffSpace * qs1 - qs2 + in;
		qm2 = qm1;
		qm1 = new_qm;
		qs2 = qs1;
		qs1 = new_qs;
	}
	float magMark = qm1 * qm1 + qm2 * qm2 - qm1 * qm2 * coeffMark;
	float magSpace = qs1 * qs1 + qs2 * qs2 - qs1 * qs2 * coeffSpace;
	bool bit = magMark > magSpace;
	handleBit(bit);
}
