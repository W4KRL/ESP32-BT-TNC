/**
 * @file afskEncodeModern.cpp
 * @date 2025-08-24
 * @brief Modern AFSK encoder implementation using latest Arduino ESP32 DAC API
 *
 * This implementation provides a complete AFSK encoder using the latest ESP32 Arduino
 * framework with improved performance, better resource management, and enhanced features.
 *
 * @author W4KRL
 * @version 2.0
 */

#include "afskEncodeModern.h"
#include <math.h>

// Static member initialization
AFSKEncoderModern* AFSKEncoderModern::_instance = nullptr;

// Global instance (optional convenience)
AFSKEncoderModern afskEncoder;

AFSKEncoderModern::AFSKEncoderModern() : 
    _timer(nullptr),
    _markWaveTable(nullptr),
    _spaceWaveTable(nullptr),
    _currentTable(0),
    _sampleIndex(0),
    _isTransmitting(false),
    _bitIndex(0),
    _totalBits(0),
    _bitStartTime(0),
    _bitBuffer(nullptr)
{
    // Initialize config with defaults
    _config = {
        .dacPin = DAC_PIN,
        .pttPin = -1,
        .pttLedPin = -1,
        .markFreq = MARK_FREQ,
        .spaceFreq = SPACE_FREQ,
        .baudRate = BAUD_RATE,
        .amplitude = AMPLITUDE,
        .samplesPerCycle = SAMPLES_PER_CYCLE,
        .initialized = false
    };
}

AFSKEncoderModern::~AFSKEncoderModern() {
    end();
}

AFSKEncoderModern::Status AFSKEncoderModern::begin(uint8_t dacPin, int8_t pttPin, int8_t pttLedPin) {
    // Validate DAC pin
    if (dacPin != 25 && dacPin != 26) {
        return Status::ERROR_INVALID_PIN;
    }

    // Store configuration
    _config.dacPin = dacPin;
    _config.pttPin = pttPin;
    _config.pttLedPin = pttLedPin;

    // Set static instance for ISR access
    _instance = this;

    // Initialize hardware
    Status status = initializeHardware();
    if (status != Status::SUCCESS) {
        return status;
    }

    // Generate waveform tables
    status = generateWaveTables();
    if (status != Status::SUCCESS) {
        cleanupResources();
        return status;
    }

    _config.initialized = true;
    return Status::SUCCESS;
}

void AFSKEncoderModern::end() {
    if (_config.initialized) {
        stopTransmission();
        cleanupResources();
        _config.initialized = false;
        _instance = nullptr;
    }
}

AFSKEncoderModern::Status AFSKEncoderModern::setParameters(uint16_t markFreq, uint16_t spaceFreq, 
                                                           uint16_t baudRate, float amplitude, 
                                                           uint8_t samplesPerCycle) {
    // Validate parameters
    if (amplitude < 0.0f || amplitude > 1.0f) {
        return Status::ERROR_INVALID_PARAMS;
    }

    if (samplesPerCycle == 0 || (samplesPerCycle & (samplesPerCycle - 1)) != 0) {
        return Status::ERROR_INVALID_PARAMS; // Must be power of 2
    }

    if (markFreq == 0 || spaceFreq == 0 || baudRate == 0) {
        return Status::ERROR_INVALID_PARAMS;
    }

    // Update configuration
    _config.markFreq = markFreq;
    _config.spaceFreq = spaceFreq;
    _config.baudRate = baudRate;
    _config.amplitude = amplitude;
    _config.samplesPerCycle = samplesPerCycle;

    // Regenerate wave tables if initialized
    if (_config.initialized) {
        return generateWaveTables();
    }

    return Status::SUCCESS;
}

AFSKEncoderModern::Status AFSKEncoderModern::transmitPacket(const uint8_t* kissFrame, size_t length) {
    if (!_config.initialized) {
        return Status::ERROR_NOT_INITIALIZED;
    }

    if (_isTransmitting) {
        return Status::ERROR_INVALID_PARAMS; // Already transmitting
    }

    if (length == 0 || kissFrame[0] != 0x00) {
        return Status::ERROR_INVALID_PARAMS; // Invalid KISS frame
    }

    // Skip KISS command byte
    const uint8_t* ax25Data = kissFrame + 1;
    size_t ax25Len = length - 1;

    // Encode AX.25 with bit stuffing (static buffer for simplicity)
    static uint8_t stuffedBuffer[1024];
    size_t stuffedLen = encodeAX25(ax25Data, ax25Len, stuffedBuffer, sizeof(stuffedBuffer));
    if (stuffedLen == 0) {
        return Status::ERROR_BUFFER_OVERFLOW;
    }

    // NRZI encode
    static uint8_t nrziBuffer[8192]; // 8 bits per byte
    size_t nrziLen = encodeNRZI(stuffedBuffer, stuffedLen, nrziBuffer, sizeof(nrziBuffer));
    if (nrziLen == 0) {
        return Status::ERROR_BUFFER_OVERFLOW;
    }

    // Start transmission
    setPTT(true);
    delay(10); // PTT settling time

    startTransmission(nrziBuffer, nrziLen);

    // Wait for transmission to complete
    while (_isTransmitting) {
        delay(1);
    }

    delay(10); // Transmission settling time
    setPTT(false);

    return Status::SUCCESS;
}

AFSKEncoderModern::Status AFSKEncoderModern::transmitBits(const uint8_t* bits, size_t length) {
    if (!_config.initialized) {
        return Status::ERROR_NOT_INITIALIZED;
    }

    if (_isTransmitting) {
        return Status::ERROR_INVALID_PARAMS;
    }

    setPTT(true);
    delay(10);

    startTransmission(bits, length);

    while (_isTransmitting) {
        delay(1);
    }

    delay(10);
    setPTT(false);

    return Status::SUCCESS;
}

const char* AFSKEncoderModern::statusToString(Status status) {
    switch (status) {
        case Status::SUCCESS: return "Success";
        case Status::ERROR_INVALID_PIN: return "Invalid DAC pin";
        case Status::ERROR_TIMER_INIT: return "Timer initialization failed";
        case Status::ERROR_DAC_INIT: return "DAC initialization failed";
        case Status::ERROR_INVALID_PARAMS: return "Invalid parameters";
        case Status::ERROR_NOT_INITIALIZED: return "Encoder not initialized";
        case Status::ERROR_BUFFER_OVERFLOW: return "Buffer overflow";
        default: return "Unknown error";
    }
}

// Private methods implementation

AFSKEncoderModern::Status AFSKEncoderModern::initializeHardware() {
    // Initialize PTT pins
    if (_config.pttPin >= 0) {
        pinMode(_config.pttPin, OUTPUT);
        digitalWrite(_config.pttPin, LOW);
    }

    if (_config.pttLedPin >= 0) {
        pinMode(_config.pttLedPin, OUTPUT);
        digitalWrite(_config.pttLedPin, LOW);
    }

    // Initialize DAC using modern Arduino ESP32 API
    pinMode(_config.dacPin, OUTPUT);
    
    // Set DAC to midpoint initially
    dacWrite(_config.dacPin, 128);

    // Initialize timer
    _timer = timerBegin(0, TIMER_DIVIDER, true);
    if (!_timer) {
        return Status::ERROR_TIMER_INIT;
    }

    timerAttachInterrupt(_timer, timerISR, true);

    return Status::SUCCESS;
}

AFSKEncoderModern::Status AFSKEncoderModern::generateWaveTables() {
    // Free existing tables
    if (_markWaveTable) {
        free(_markWaveTable);
        _markWaveTable = nullptr;
    }
    if (_spaceWaveTable) {
        free(_spaceWaveTable);
        _spaceWaveTable = nullptr;
    }

    // Allocate new tables
    _markWaveTable = (uint8_t*)malloc(_config.samplesPerCycle);
    _spaceWaveTable = (uint8_t*)malloc(_config.samplesPerCycle);

    if (!_markWaveTable || !_spaceWaveTable) {
        cleanupResources();
        return Status::ERROR_DAC_INIT;
    }

    // Generate sine wave tables
    const float amplitude = _config.amplitude;
    const uint8_t midpoint = 128; // 8-bit DAC midpoint

    for (uint8_t i = 0; i < _config.samplesPerCycle; i++) {
        float angle = 2.0f * PI * i / _config.samplesPerCycle;
        float sineValue = sin(angle);
        
        // Scale to DAC range with amplitude control
        uint8_t dacValue = (uint8_t)(midpoint + (amplitude * 127.0f * sineValue));
        
        _markWaveTable[i] = dacValue;
        _spaceWaveTable[i] = dacValue; // Same waveform, different timing
    }

    return Status::SUCCESS;
}

void AFSKEncoderModern::cleanupResources() {
    if (_timer) {
        timerEnd(_timer);
        _timer = nullptr;
    }

    if (_markWaveTable) {
        free(_markWaveTable);
        _markWaveTable = nullptr;
    }

    if (_spaceWaveTable) {
        free(_spaceWaveTable);
        _spaceWaveTable = nullptr;
    }
}

size_t AFSKEncoderModern::encodeAX25(const uint8_t* input, size_t inputLen, uint8_t* output, size_t maxOutputLen) {
    const uint8_t KISS_FLAG = 0x7E;
    size_t outIndex = 0;

    // Check buffer space
    if (maxOutputLen < 2) return 0; // At least need room for flags

    output[outIndex++] = KISS_FLAG;

    uint8_t buf = 0;
    int count = 0, ones = 0;

    for (size_t i = 0; i < inputLen && outIndex < maxOutputLen - 1; i++) {
        for (int j = 0; j < 8; j++) {
            bool bit = input[i] & (1 << j);
            
            if (bit) {
                ones++;
            } else {
                ones = 0;
            }

            buf |= (bit ? 1 : 0) << count++;

            // Bit stuffing: insert 0 after five consecutive 1s
            if (ones == 5) {
                if (outIndex >= maxOutputLen - 1) return 0; // Buffer full
                buf |= 0 << count++;
                ones = 0;
            }

            if (count == 8) {
                output[outIndex++] = buf;
                buf = 0;
                count = 0;
                if (outIndex >= maxOutputLen - 1) return 0; // Buffer full
            }
        }
    }

    if (count > 0) {
        output[outIndex++] = buf;
    }

    if (outIndex < maxOutputLen) {
        output[outIndex++] = KISS_FLAG;
    }

    return outIndex;
}

size_t AFSKEncoderModern::encodeNRZI(const uint8_t* input, size_t inputLen, uint8_t* output, size_t maxOutputLen) {
    bool lastBit = true;
    size_t outIndex = 0;

    for (size_t i = 0; i < inputLen && outIndex < maxOutputLen; i++) {
        for (int j = 0; j < 8 && outIndex < maxOutputLen; j++) {
            bool bit = input[i] & (1 << j);
            
            // NRZI: 0 causes transition, 1 causes no change
            if (!bit) {
                lastBit = !lastBit;
            }
            
            output[outIndex++] = lastBit ? 1 : 0;
        }
    }

    return outIndex;
}

void AFSKEncoderModern::setPTT(bool enable) {
    if (_config.pttPin >= 0) {
        digitalWrite(_config.pttPin, enable ? HIGH : LOW);
    }
    if (_config.pttLedPin >= 0) {
        digitalWrite(_config.pttLedPin, enable ? HIGH : LOW);
    }
}

void AFSKEncoderModern::startTransmission(const uint8_t* bits, size_t bitCount) {
    _bitBuffer = bits;
    _totalBits = bitCount;
    _bitIndex = 0;
    _sampleIndex = 0;
    _isTransmitting = true;
    _bitStartTime = micros();

    // Set initial frequency based on first bit
    _currentTable = _bitBuffer[0] ? 0 : 1; // 1 = mark (1200Hz), 0 = space (2200Hz)

    // Calculate timer interval for current frequency
    uint16_t freq = _currentTable == 0 ? _config.markFreq : _config.spaceFreq;
    uint64_t ticksPerSample = TIMER_FREQ / (freq * _config.samplesPerCycle);

    timerAlarmWrite(_timer, ticksPerSample, true);
    timerAlarmEnable(_timer);
}

void AFSKEncoderModern::stopTransmission() {
    _isTransmitting = false;
    timerAlarmDisable(_timer);
    
    // Set DAC to midpoint
    dacWrite(_config.dacPin, 128);
}

void IRAM_ATTR AFSKEncoderModern::timerISR() {
    if (_instance) {
        _instance->handleTimerInterrupt();
    }
}

void IRAM_ATTR AFSKEncoderModern::handleTimerInterrupt() {
    if (!_isTransmitting || !_bitBuffer) {
        return;
    }

    // Output current sample
    uint8_t* currentWaveTable = _currentTable == 0 ? _markWaveTable : _spaceWaveTable;
    dacWrite(_config.dacPin, currentWaveTable[_sampleIndex]);

    _sampleIndex++;
    if (_sampleIndex >= _config.samplesPerCycle) {
        _sampleIndex = 0;
    }

    // Check if it's time to advance to next bit
    uint64_t bitDuration = 1000000ULL / _config.baudRate; // microseconds per bit
    uint64_t elapsed = micros() - _bitStartTime;

    if (elapsed >= bitDuration) {
        _bitIndex++;
        if (_bitIndex >= _totalBits) {
            // Transmission complete
            _isTransmitting = false;
            timerAlarmDisable(_timer);
            dacWrite(_config.dacPin, 128); // Set to midpoint
            return;
        }

        // Start next bit
        _bitStartTime = micros();
        _currentTable = _bitBuffer[_bitIndex] ? 0 : 1; // 1 = mark, 0 = space

        // Update timer frequency for new bit
        uint16_t freq = _currentTable == 0 ? _config.markFreq : _config.spaceFreq;
        uint64_t ticksPerSample = TIMER_FREQ / (freq * _config.samplesPerCycle);
        timerAlarmWrite(_timer, ticksPerSample, true);
    }
}
