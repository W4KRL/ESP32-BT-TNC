#ifndef WAVE_TABLES_H
#define WAVE_TABLES_H

#include <Arduino.h>

bool loadTables();			   // Load wave tables from NVS
void generateAndStoreTables(); // Generate and store wave tables in NVS

extern uint8_t sine1200[];	  // Sine wave table for 1200 Hz
extern uint8_t sine2200[];	  // Sine wave table for 2200 Hz
extern const char *ready_key; // Key to check if tables are ready
extern const int TABLE_LEN;	  // Length of the wave tables
extern const int SAMPLE_RATE; // Sample rate for AFSK modulation

#endif