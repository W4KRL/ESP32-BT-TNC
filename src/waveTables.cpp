#include "waveTables.h"

#include <Arduino.h>
#include <Preferences.h>

Preferences prefs;

const int TABLE_LEN = 64;	  // Length of the wave tables
const int SAMPLE_RATE = 8000; // Sample rate for AFSK modulation

uint8_t sine1200[TABLE_LEN];
uint8_t sine2200[TABLE_LEN];

const char *ready_key = "ready"; // Key to check if tables are ready

// Generate and store wave tables in NVS (Preferences)
void generateAndStoreTables()
{
	prefs.begin("afsk", false);

	for (int i = 0; i < TABLE_LEN; i++)
	{
		float angle1200 = 2 * PI * i * 1200.0 / SAMPLE_RATE;
		float angle2200 = 2 * PI * i * 2200.0 / SAMPLE_RATE;
		// TODO: Adjust amplitude and offset as needed
		sine1200[i] = 128 + 80 * sin(angle1200); // amplitude example: 80
		sine2200[i] = 128 + 80 * sin(angle2200);
	}

	// Store tables as binary blobs in NVS
	prefs.putBytes("sine1200", sine1200, TABLE_LEN);
	prefs.putBytes("sine2200", sine2200, TABLE_LEN);

	prefs.putBool(ready_key, true);
	prefs.end();
}

// Load tables from NVS; returns true if successful
bool loadTables()
{
	prefs.begin("afsk", true); // read-only
	if (!prefs.getBool(ready_key, false))
	{
		prefs.end();
		return false;
	}
	size_t len1 = prefs.getBytes("sine1200", sine1200, TABLE_LEN);
	size_t len2 = prefs.getBytes("sine2200", sine2200, TABLE_LEN);
	prefs.end();
	return (len1 == TABLE_LEN && len2 == TABLE_LEN);
}
