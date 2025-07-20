#include <Arduino.h>
#include "btFunctions.h" // Include Bluetooth functions
#include "kissFunctions.h"

// KISS protocol special characters
#define FEND 0xC0  // Frame End
#define FESC 0xDB  // Frame Escape
#define TFEND 0xDC // Transposed FEND
#define TFESC 0xDD // Transposed FESC

const uint8_t KISS_FRAME_FLAG = 0x7E; // KISS frame flag

void sendKISSpacket(uint8_t *data, size_t len)
{
	// Serial.write(FEND);
	// Serial.write(0x00);
	BTSerial.write(FEND);
	BTSerial.write(0x00); // Indicates start of KISS data frame

	for (size_t i = 0; i < len; i++)
	{
		if (data[i] == FEND)
		{
			// Serial.write(FESC);
			// Serial.write(TFEND);
			BTSerial.write(FESC);
			BTSerial.write(TFEND);
		}
		else if (data[i] == FESC)
		{
			// Serial.write(FESC);
			// Serial.write(TFESC);
			BTSerial.write(FESC);
			BTSerial.write(TFESC);
		}
		else
		{
			// Serial.write(data[i]);
			BTSerial.write(data[i]);
		}
	}
	// Serial.write(FEND);
	BTSerial.write(FEND);
}

