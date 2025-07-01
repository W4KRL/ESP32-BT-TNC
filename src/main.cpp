/**
 * @file main.cpp
 * @brief ESP32 KISS TNC implementation with Bluetooth and USB Serial support.
 *
 * This program implements a KISS (Keep It Simple Stupid) TNC (Terminal Node Controller)
 * for the ESP32 platform, supporting both USB Serial and Bluetooth Serial interfaces.
 * It is designed for amateur radio applications, enabling packet radio communication
 * using the KISS protocol over AFSK (Audio Frequency Shift Keying) modulation.
 *
 * Features:
 * - Sends and receives KISS frames over USB Serial and Bluetooth Serial.
 * - Encodes and decodes KISS special characters (FEND, FESC, TFEND, TFESC).
 * - Generates AFSK tones for packet transmission (simplified example).
 * - Detects incoming AFSK audio and sends decoded packets as KISS frames (simplified).
 * - Controls PTT (Push-to-Talk) for radio transmission.
 *
 * Pin Definitions:
 * - PIN_ATR (25): Audio Frequency Transmit (not used in this example).
 * - PIN_AFR (34): Audio Frequency Receive (used for analog input and tone output).
 * - PIN_PTT (13): Push-to-Talk control pin.
 *
 * Usage:
 * - Connect the ESP32 to a radio transceiver using the defined pins.
 * - Communicate with the TNC using a terminal or application over USB or Bluetooth.
 * - Use KISS protocol frames for packet radio communication.
 *
 * Notes:
 * - The AFSK modulation/demodulation is highly simplified and intended for demonstration.
 * - Proper AFSK implementation requires more advanced signal processing.
 * - Use at your own risk; modifications may be required for specific hardware setups.
 *
 * @author AI generated code modified by 2E0UMR, adapted to PlatformIO by W4KRL
 * @see https://uhpowerup.com/
 * @see https://hackaday.io/project/202603-esp32-kiss-tnc
 */

#include <Arduino.h>         // Include the Arduino core for ESP32
#include "BluetoothSerial.h" // use the built-in BluetoothSerial library
BluetoothSerial BTSerial;    // Bluetooth KISS Interface
bool useBluetooth = false;   // Flag to toggle between USB Serial and Bluetooth Serial

// Pin definitions for transceiver interface
#define PIN_ATR 25 // Audio Frequency Transmit
#define PIN_AFR 34 // Audio Frequency Receive
#define PIN_PTT 13 // Push-to-Talk control pin W4KRL version

// KISS protocol special characters
#define FEND 0xC0  // Frame End
#define FESC 0xDB  // Frame Escape
#define TFEND 0xDC // Transposed FEND
#define TFESC 0xDD // Transposed FESC

/**
 * @brief Initializes serial communication, Bluetooth, and PTT pin.
 *
 * This function performs the following setup tasks:
 * - Initializes the USB serial port at 115200 baud for debugging output.
 * - Starts Bluetooth serial communication with the device name "ESP32_KISS_TNC".
 * - Configures the PTT (Push-To-Talk) pin as an output.
 * - Sets the PTT pin to LOW to ensure the transmitter is not active at startup.
 * - Prints a ready message to the serial monitor.
 */
void setup()
{
  Serial.begin(115200);             // USB Serial for debugging
  BTSerial.begin("ESP32_KISS_TNC"); // Bluetooth name
  pinMode(PIN_PTT, OUTPUT);         // Set PTT pin as output
  digitalWrite(PIN_PTT, LOW);       // Ensure PTT is low initially (not transmitting)
  Serial.println("ESP32 KISS TNC Ready");
}

/**
 * @brief Sends a KISS-encoded packet over both Serial and BTSerial interfaces.
 *
 * This function takes a data buffer and its length, applies KISS protocol
 * framing and escaping, and writes the encoded packet to both Serial and
 * BTSerial outputs. The KISS protocol uses special frame delimiter (FEND)
 * and escape (FESC) bytes, which are escaped within the payload as needed.
 *
 * @param data Pointer to the data buffer to send.
 * @param len  Length of the data buffer in bytes.
 */
void sendKISSPacket(uint8_t *data, size_t len)
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

/**
 * @brief Transmits a packet using Audio Frequency-Shift Keying (AFSK) modulation.
 *
 * This function keys the transmitter, then iterates through the provided packet,
 * generating either a 1200 Hz or 2200 Hz tone for each bit in the packet, depending
 * on the least significant bit of each byte. Each tone is played for 10 milliseconds.
 * After transmission, the transmitter is held active briefly before being unkeyed.
 *
 * @param packet Pointer to the data packet to transmit.
 * @param len    Length of the packet in bytes.
 */
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

/**
 * @brief Reads an analog signal from the AFSK input pin and detects if it exceeds a threshold.
 *
 * This function samples the analog value from the specified AFSK input pin (PIN_AFR).
 * If the sampled value is greater than 2000, it simulates the reception of an AFSK packet
 * by creating a predefined packet and sending it using the sendKISSPacket function.
 *
 * The received packet is hardcoded for demonstration purposes and contains both header and payload data.
 */
void receiveAFSK()
{
  int sample = analogRead(PIN_AFR);
  if (sample > 2000) // Threshold for detecting signal
  {
    uint8_t receivedPacket[] = {0x82, 0xA6, 0x40, 0x61, 0xE0, 0x03, 0xF0, 'H', 'e', 'l', 'l', 'o'};
    sendKISSPacket(receivedPacket, sizeof(receivedPacket));
  }
}

/**
 * @brief Main loop function for handling KISS frames and AFSK processing.
 *
 * This function continuously checks for incoming KISS frames on both the USB Serial
 * and Bluetooth Serial interfaces. When data is available, it reads the frame and
 * transmits it using AFSK modulation. Additionally, it processes any incoming audio
 * signals by calling receiveAFSK().
 *
 * - Checks USB Serial for available KISS frames and transmits them via AFSK.
 * - Checks Bluetooth Serial for available KISS frames and transmits them via AFSK.
 * - Continuously processes incoming audio for AFSK reception.
 */
void loop()
{
  // Check USB Serial for KISS frames
  if (Serial.available())
  {
    uint8_t kissFrame[256];
    size_t len = BTSerial.readBytes(kissFrame, sizeof(kissFrame));
    transmitAFSK(kissFrame, len);
  }

  // Check Bluetooth Serial for KISS frames
  if (BTSerial.available())
  {
    uint8_t kissFrame[256];
    size_t len = BTSerial.readBytes(kissFrame, sizeof(kissFrame));
    transmitAFSK(kissFrame, len);
  }

  receiveAFSK(); // Process incoming audio
}
