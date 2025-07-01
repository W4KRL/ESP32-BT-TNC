#include <Arduino.h>
#include "btKISSfunctions.h"

BluetoothSerial BTSerial; // Bluetooth KISS Interface

// Pin definitions for transceiver interface
int PIN_AFT = 25; // Audio Frequency Transmit
int PIN_AFR = 34; // Audio Frequency Receive
int PIN_PTT = 13; // Push-to-Talk control pin W4KRL version

// KISS protocol special characters
#define FEND 0xC0  // Frame End
#define FESC 0xDB  // Frame Escape
#define TFEND 0xDC // Transposed FEND
#define TFESC 0xDD // Transposed FESC

void setupBluetooth()
{
  BTSerial.begin("ESP32_KISS_TNC"); // Bluetooth name
  pinMode(PIN_PTT, OUTPUT);         // Set PTT pin as output
  digitalWrite(PIN_PTT, LOW);       // Ensure PTT is low initially (not transmitting)
  Serial.println("ESP32 KISS TNC Ready");
}	

void sendKISSpacket(uint8_t *data, size_t len)
{
  Serial.write(FEND);
  BTSerial.write(FEND);

  for (size_t i = 0; i < len; i++)
  {
    if (data[i] == FEND)
    {
      Serial.write(FESC);
      Serial.write(TFEND);
      BTSerial.write(FESC);
      BTSerial.write(TFEND);
    }
    else if (data[i] == FESC)
    {
      Serial.write(FESC);
      Serial.write(TFESC);
      BTSerial.write(FESC);
      BTSerial.write(TFESC);
    }
    else
    {
      Serial.write(data[i]);
      BTSerial.write(data[i]);
    }
  }
  Serial.write(FEND);
  BTSerial.write(FEND);
}

void transmitAFSK(uint8_t *packet, size_t len)
{
  digitalWrite(PIN_PTT, HIGH);
  delay(50); // Small delay for radio keying

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