/**
 * @file afskEncodeModern.h
 * @date 2025-08-24
 * @brief Modern AFSK (Audio Frequency-Shift Keying) encoder using latest Arduino ESP32 DAC API
 *
 * This header provides a modernized implementation of AFSK encoding for AX.25 frame transmission
 * using the latest Arduino ESP32 framework DAC API. It features improved performance, better
 * resource management, and enhanced configuration options.
 *
 * Key Improvements:
 * - Uses latest ESP32 Arduino DAC API (dacWrite() function)
 * - Configurable DAC channel and resolution
 * - Improved timer management with proper cleanup
 * - Enhanced error handling and status reporting
 * - Support for different sample rates and frequencies
 * - Memory-efficient waveform generation
 * - Thread-safe implementation
 *
 * Hardware Requirements:
 * - ESP32 with DAC capability (GPIO25 or GPIO26)
 * - PTT control pin for transmitter keying
 * - Optional PTT LED indicator
 *
 * Usage:
 * 1. Call afskEncoder.begin() during setup to initialize hardware
 * 2. Configure frequencies and amplitude with setParameters()
 * 3. Use transmitPacket() to send AX.25 frames
 * 4. Call afskEncoder.end() when done to clean up resources
 *
 * @author W4KRL
 * @version 2.0
 * @see https://docs.espressif.com/projects/arduino-esp32/en/latest/api/dac.html
 */

#ifndef AFSK_ENCODE_MODERN_H
#define AFSK_ENCODE_MODERN_H

#include <Arduino.h>

/**
 * @brief Modern AFSK Encoder class using latest ESP32 DAC API
 */
class AFSKEncoderModern {
public:
    // Configuration constants
    static constexpr uint8_t DEFAULT_DAC_PIN = 25;          // GPIO25 (DAC1)
    static constexpr uint16_t DEFAULT_MARK_FREQ = 1200;     // Mark frequency (Hz)
    static constexpr uint16_t DEFAULT_SPACE_FREQ = 2200;    // Space frequency (Hz)
    static constexpr uint16_t DEFAULT_BAUD_RATE = 1200;     // Baud rate (bps)
    static constexpr uint8_t DEFAULT_SAMPLES_PER_CYCLE = 32; // Power of 2
    static constexpr uint8_t DEFAULT_DAC_RESOLUTION = 8;    // 8-bit DAC
    static constexpr float DEFAULT_AMPLITUDE = 0.8f;        // 80% of max amplitude

    // Error codes
    enum class Status {
        SUCCESS = 0,
        ERROR_INVALID_PIN,
        ERROR_TIMER_INIT,
        ERROR_DAC_INIT,
        ERROR_INVALID_PARAMS,
        ERROR_NOT_INITIALIZED,
        ERROR_BUFFER_OVERFLOW
    };

    /**
     * @brief Constructor
     */
    AFSKEncoderModern();

    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~AFSKEncoderModern();

    /**
     * @brief Initialize the AFSK encoder with default settings
     * @param dacPin DAC output pin (GPIO25 or GPIO26)
     * @param pttPin PTT control pin
     * @param pttLedPin Optional PTT LED indicator pin
     * @return Status code
     */
    Status begin(uint8_t dacPin = DEFAULT_DAC_PIN, 
                 int8_t pttPin = -1, 
                 int8_t pttLedPin = -1);

    /**
     * @brief Clean up resources and disable encoder
     */
    void end();

    /**
     * @brief Configure AFSK parameters
     * @param markFreq Mark frequency in Hz (typically 1200)
     * @param spaceFreq Space frequency in Hz (typically 2200)
     * @param baudRate Baud rate in bits per second
     * @param amplitude Amplitude (0.0 to 1.0)
     * @param samplesPerCycle Samples per waveform cycle (power of 2)
     * @return Status code
     */
    Status setParameters(uint16_t markFreq = DEFAULT_MARK_FREQ,
                        uint16_t spaceFreq = DEFAULT_SPACE_FREQ,
                        uint16_t baudRate = DEFAULT_BAUD_RATE,
                        float amplitude = DEFAULT_AMPLITUDE,
                        uint8_t samplesPerCycle = DEFAULT_SAMPLES_PER_CYCLE);

    /**
     * @brief Transmit an AX.25 packet using AFSK modulation
     * @param kissFrame KISS frame data (first byte should be 0x00)
     * @param length Length of the KISS frame
     * @return Status code
     */
    Status transmitPacket(const uint8_t* kissFrame, size_t length);

    /**
     * @brief Transmit raw bits for testing purposes
     * @param bits Array of bits (0 or 1)
     * @param length Number of bits to transmit
     * @return Status code
     */
    Status transmitBits(const uint8_t* bits, size_t length);

    /**
     * @brief Check if encoder is currently transmitting
     * @return true if transmitting, false otherwise
     */
    bool isTransmitting() const { return _isTransmitting; }

    /**
     * @brief Get current configuration
     */
    struct Config {
        uint8_t dacPin;
        int8_t pttPin;
        int8_t pttLedPin;
        uint16_t markFreq;
        uint16_t spaceFreq;
        uint16_t baudRate;
        float amplitude;
        uint8_t samplesPerCycle;
        bool initialized;
    };

    const Config& getConfig() const { return _config; }

    /**
     * @brief Convert status code to human-readable string
     * @param status Status code to convert
     * @return String description of the status
     */
    static const char* statusToString(Status status);

private:
    // Configuration
    Config _config;

    // Hardware timer
    hw_timer_t* _timer;
    static constexpr uint8_t TIMER_DIVIDER = 8;    // For 0.1µs resolution
    static constexpr uint64_t TIMER_FREQ = APB_CLK_FREQ / TIMER_DIVIDER;

    // Waveform tables
    uint8_t* _markWaveTable;
    uint8_t* _spaceWaveTable;
    volatile uint8_t _currentTable;  // 0 = mark, 1 = space
    volatile uint16_t _sampleIndex;
    
    // Transmission state
    volatile bool _isTransmitting;
    volatile size_t _bitIndex;
    volatile size_t _totalBits;
    volatile uint64_t _bitStartTime;
    const uint8_t* _bitBuffer;

    // Internal methods
    Status initializeHardware();
    Status generateWaveTables();
    void cleanupResources();
    
    size_t encodeAX25(const uint8_t* input, size_t inputLen, uint8_t* output, size_t maxOutputLen);
    size_t encodeNRZI(const uint8_t* input, size_t inputLen, uint8_t* output, size_t maxOutputLen);
    
    void setPTT(bool enable);
    void startTransmission(const uint8_t* bits, size_t bitCount);
    void stopTransmission();

    // Timer interrupt handler (must be static for C callback)
    static void IRAM_ATTR timerISR();
    void IRAM_ATTR handleTimerInterrupt();

    // Static instance for ISR access
    static AFSKEncoderModern* _instance;
};

// Global instance for easy access (optional)
extern AFSKEncoderModern afskEncoder;

#endif // AFSK_ENCODE_MODERN_H
