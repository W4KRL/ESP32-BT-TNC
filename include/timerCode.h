#ifndef TIMER_CODE_H
#define TIMER_CODE_H

#include <Arduino.h>
void populateWaveTable(float amplitude);			// Function to populate the waveTable array with one complete cycle of sine wave
void setupCallbackTimer(uint64_t ticks_per_sample); // Function to set up the timer to call the onTimer() function at the desired sample rate

extern const unsigned int SAMPLES_PER_CYCLE; // Number of samples per cycle of the waveform (power of 2, e.g., 32, 64, 128, etc.)
extern float frequency;						 // Desired frequency (Hz) of the output waveform
extern float amplitude;						 // Amplitude of the waveform (0.0 to 1.0, where 1.0 is the maximum DAC value)
extern uint64_t ticksPerSample;				 // Timer ticks per cycle (set at run time)
extern const uint64_t TICKS_PER_S;			 // The timer has a resolution of 1 microsecond

#endif // TIMER_CODE_H