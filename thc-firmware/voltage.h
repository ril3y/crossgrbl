/*
 * voltage.h - Arc Voltage Sensing Module
 *
 * Reads arc voltage via ADC with rolling average filtering.
 */

#ifndef VOLTAGE_H
#define VOLTAGE_H

#include <Arduino.h>
#include "config.h"

class Voltage {
public:
    Voltage();

    // Initialize ADC
    void begin();

    // Update voltage reading (call frequently from main loop)
    void update();

    // Get current filtered voltage
    float getVoltage();

    // Get raw ADC value (for debugging)
    uint16_t getRawADC();

    // Check if arc is established (above threshold)
    bool isArcOk();

    // Check if arc is lost (below threshold)
    bool isArcLost();

    // Smart voltage sampling
    void startSampling();
    void stopSampling();
    float getSampledVoltage();
    bool isSamplingComplete();

private:
    // Rolling average buffer
    uint16_t samples[VOLTAGE_SAMPLES];
    uint8_t sampleIndex;
    uint32_t sampleSum;
    bool bufferFilled;

    // Timing
    uint32_t lastReadTime;

    // Cached voltage value
    float currentVoltage;

    // Smart sampling
    bool sampling;
    uint8_t smartSampleCount;
    float smartSampleSum;
    uint32_t samplingStartTime;

    // Convert ADC to voltage
    float adcToVoltage(uint16_t adc);
};

// Global instance
extern Voltage voltage;

#endif // VOLTAGE_H
