#include "timerCode.h"

#include <Arduino.h>
#include "driver/dac.h"
#include "configuration.h"

// Constants for the ESP32 timer and DAC
// The ESP32 timer runs at 80 MHz, so we need to divide it down for our use
// The Arduino library includes a macro for the APB clock frequency
#define TIMER_DIVIDER 8                                    // for 0.1 uS resolution divide ESP32 80 MHz timer by 8
const uint64_t TICKS_PER_S = APB_CLK_FREQ / TIMER_DIVIDER; // set the timer resolution to 0.1 uS per tick (10 MHz)
#define DAC_CHANNEL DAC_CHANNEL_1                          // the waveform output pin. (e.g., DAC_CHANNEL_1 (25) or DAC_CHANNEL_2 (26))
#define MAX_DAC_VALUE 255                                  // the maximum ESP32 DAC value
uint64_t ticksPerSample = 0;                               // timer ticks per cycle (set at run time)
;                                                          // Timer handle for the ESP32 timer
hw_timer_t *timer = NULL;                                  // instantiate the timer

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
    unsigned int value = round(MAX_DAC_VALUE / 2.0 + amplitude * MAX_DAC_VALUE / 2.0 * sin(angleInRadians));
    waveTable[i] = value;
    // Serial.printf("step: %d, radians: %.6f, value: %d\n", i, angleInRadians, value);
  }
}

/**
 * The function generates and outputs the sine wave to the DAC channel.
 * It is called periodically by the timer.
 * This function:
 *  1) gets a value of the waveform from an array (one sample step's value)
 *  2) outputs the value to the DAC channel
 *  3) advances the array index or resets it to zero (the sample's step number)
 */
// void IRAM_ATTR onTimer()
void onTimer()
{
  static unsigned currentSampleIdx = 0;                    // static variable to keep track of the current sample index
  unsigned int sampleValue = waveTable[currentSampleIdx];  // get the waveform value from the array
  dac_output_voltage(DAC_CHANNEL, sampleValue);            // output the voltage to the DAC_CHANNEL
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
   dac_output_enable(DAC_CHANNEL);                               // Enable DAC output before starting the timer
  // The rate at which the timer will call the onTimer() function
  // is ticks_per_sample times the timer resolution (0.1 uS)
  // Timer resolution is TIMER_DIVIDER / APB_CLK_FREQ
  int timerId = 0;        // use ESP32 timer 0
  bool countUp = true;    // count up from 0 to the alarm value
  bool autoreload = true; // auto-reload the timer after each callback
  bool edge = true;       // edge-triggered interrupt

  timer = timerBegin(timerId, TIMER_DIVIDER, countUp);
  timerAttachInterrupt(timer, onTimer, edge);
  timerAlarmWrite(timer, ticks_per_sample, autoreload);
  timerAlarmEnable(timer);
}

/**
 * @brief Prints the settings to the terminal
 *
 */
void printSettings()
{
  Serial.printf("\n%s\n","=======================================================");
  Serial.printf("Frequency         : %.f Hz \n", frequency);
  Serial.printf("Amplitude         : %.1f (0.0 to 1.0) \n", amplitude);
  Serial.printf("Samples Per Cycle : %d samples per cycle \n", SAMPLES_PER_CYCLE);
  Serial.printf("Sample Rate       : %.f samples per second \n", frequency * SAMPLES_PER_CYCLE);
  Serial.printf("Ticks Per Sample  : %d ticks \n", ticksPerSample);
  Serial.printf("%s\n", "=======================================================");
}

