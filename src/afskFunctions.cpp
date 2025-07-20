#include "afskFunctions.h"

#include <Arduino.h>
#include "kissFunctions.h"
#include "driver/ledc.h"
#include "configuration.h"

void setupAFSK()
{
  pinMode(TX_PIN, OUTPUT);    // Set TX pin as output
  pinMode(RX_PIN, INPUT);     // Set RX pin as input for analog read
  pinMode(PTT_PIN, OUTPUT);   // Set PTT pin as output
  digitalWrite(PTT_PIN, LOW); // Ensure PTT is low initially (not transmitting)
  digitalWrite(PTT_LED, LOW); // Ensure PTT LED is off initially

 // Arduino-style LEDC setup for square wave output
  ledcSetup(0, 1200, 8);        // channel 0, 1200 Hz, 8-bit resolution
  ledcAttachPin(TX_PIN, 0);     // attach TX_PIN to channel 0
  ledcWrite(0, 127);            // 50% duty cycle (127/255 for 8-bit)
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
    // Change frequency for each bit using Arduino-style API
    ledcSetup(0, bits[i] ? 1200 : 2200, 8); // channel 0, freq, 8-bit resolution
    ledcWrite(0, 127);                      // 50% duty cycle
    delayMicroseconds(BIT_DURATION_US);
  }
  ledcWrite(0, 0); // Turn off output after sending
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
