#include <Arduino.h>
#include "afskFunctions.h"
#include "btFunctions.h"
#include "kissFunctions.h"

// Pin definitions for transceiver interface
int PIN_ATR = 25; // Audio from radio
int PIN_AFR = 34; // Audio to radio
int PIN_PTT = 13; //! Push-to-Talk control pin W4KRL version

void setupAFSK()
{
  pinMode(PIN_ATR, OUTPUT);   // Set AFT pin as output
  pinMode(PIN_AFR, INPUT);    // Set AFR pin as input for analog read
  pinMode(PIN_PTT, OUTPUT);   // Set PTT pin as output
  digitalWrite(PIN_PTT, LOW); // Ensure PTT is low initially (not transmitting)
}

void transmitAFSK(uint8_t *packet, size_t len)
{
  digitalWrite(PIN_PTT, HIGH); // Set PTT high to enable transmission
  delay(50);                   // Small delay for radio keying

  for (size_t i = 0; i < len; i++)
  {
    // Generate 1200 Hz or 2200 Hz based on packet bits
    tone(PIN_AFR, (packet[i] & 1) ? 2200 : 1200, 10);
    delay(10);
  }

  delay(50); // Hold PIN_PTT briefly
  digitalWrite(PIN_PTT, LOW);
}

void receiveAFSK()
{
  int sample = analogRead(PIN_AFR);
  if (sample > 2000) // Threshold for detecting signal
  {
    uint8_t receivedPacket[] = {0x82, 0xA6, 0x40, 0x61, 0xE0, 0x03, 0xF0, 'H', 'e', 'l', 'l', 'o'};
    sendKISSpacket(receivedPacket, sizeof(receivedPacket));
  }
}