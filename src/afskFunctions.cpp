#include "afskFunctions.h"

#include <Arduino.h>
#include "kissFunctions.h"
#include "driver/ledc.h"
#include "configuration.h"

void setupAFSK()
{
  pinMode(TX_PIN, OUTPUT);    // Set AFT pin as output
  pinMode(RX_PIN, INPUT);     // Set AFR pin as input for analog read
  pinMode(PTT_PIN, OUTPUT);   // Set PTT pin as output
  digitalWrite(PTT_PIN, LOW); // Ensure PTT is low initially (not transmitting
  digitalWrite(PTT_LED, LOW); // Ensure PTT LED is off initially

  // LEDC Timer config
  ledc_timer_config_t ledc_timer = {
      LEDC_HIGH_SPEED_MODE, // speed_mode
      LEDC_TIMER_8_BIT,     // duty_resolution
      LEDC_TIMER_0,         // timer_num
      1200,                 // freq_hz
      LEDC_AUTO_CLK         // clk_cfg
  };
  ledc_timer_config(&ledc_timer);

  // LEDC Channel config
  ledc_channel_config_t ledc_channel = {
      TX_PIN,               // gpio_num
      LEDC_HIGH_SPEED_MODE, // speed_mode
      LEDC_CHANNEL_0,       // channel
      LEDC_INTR_DISABLE,    // intr_type
      LEDC_TIMER_0,         // timer_sel
      127,                  // duty
      0                     // hpoint
  };
  ledc_channel_config(&ledc_channel);
}

size_t ax25Encode(uint8_t *input, size_t len, uint8_t *output)
{
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

void afskSend(uint8_t *bits, size_t len)
{
  const unsigned long BIT_DURATION_US = 833;
  for (size_t i = 0; i < len; i++)
  {
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, bits[i] ? 1200 : 2200);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 127);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    delayMicroseconds(BIT_DURATION_US);
  }
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

void transmitAX25(uint8_t *kissFrame, size_t len)
{
  if (len == 0 || kissFrame[0] != 0x00)
  {
    return;
  }
  uint8_t *ax25 = kissFrame + 1;
  size_t ax25Len = len - 1;
  uint8_t stuffed[600], nrzi[600];
  size_t stuffedLen = ax25Encode(ax25, ax25Len, stuffed);
  size_t nrziLen = nrziEncode(stuffed, stuffedLen, nrzi);
  digitalWrite(PTT_PIN, HIGH);
  digitalWrite(PTT_LED, HIGH);
  delay(50);
  afskSend(nrzi, nrziLen);
  delay(50);
  digitalWrite(PTT_PIN, LOW);
  digitalWrite(PTT_LED, LOW);
}
