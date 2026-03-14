/*
 * voltage.cpp - Arc Voltage Sensing Implementation
 */

#include "voltage.h"
#include "settings.h"

// Global instance
Voltage voltage;

Voltage::Voltage() {
    sampleIndex = 0;
    sampleSum = 0;
    bufferFilled = false;
    lastReadTime = 0;
    currentVoltage = 0;
    sampling = false;
    smartSampleCount = 0;
    smartSampleSum = 0;
    samplingStartTime = 0;

    memset(samples, 0, sizeof(samples));
}

void Voltage::begin() {
    pinMode(PIN_ARC_VOLTAGE, INPUT);

    // Set ADC reference to default (5V)
    analogReference(DEFAULT);

    // Prime the filter with initial readings
    for (int i = 0; i < VOLTAGE_SAMPLES; i++) {
        samples[i] = analogRead(PIN_ARC_VOLTAGE);
        sampleSum += samples[i];
    }
    bufferFilled = true;
    sampleIndex = 0;

    currentVoltage = adcToVoltage(sampleSum / VOLTAGE_SAMPLES);
}

void Voltage::update() {
    uint32_t now = micros();

    // Only read at configured interval
    if (now - lastReadTime < VOLTAGE_READ_INTERVAL_US) {
        return;
    }
    lastReadTime = now;

    // Read new sample
    uint16_t newSample = analogRead(PIN_ARC_VOLTAGE);

    // Update rolling average
    sampleSum -= samples[sampleIndex];
    samples[sampleIndex] = newSample;
    sampleSum += newSample;

    sampleIndex++;
    if (sampleIndex >= VOLTAGE_SAMPLES) {
        sampleIndex = 0;
        bufferFilled = true;
    }

    // Calculate filtered voltage
    currentVoltage = adcToVoltage(sampleSum / VOLTAGE_SAMPLES);

    // Handle smart sampling if active
    if (sampling) {
        uint32_t elapsed = millis() - samplingStartTime;

        // Wait for sample delay before collecting
        if (elapsed >= settings.data.sampleDelay) {
            if (smartSampleCount < settings.data.sampleSize) {
                smartSampleSum += currentVoltage;
                smartSampleCount++;
            }
        }
    }

#ifdef DEBUG_VOLTAGE
    static uint32_t lastDebug = 0;
    if (millis() - lastDebug > 500) {
        Serial.print(F("V: "));
        Serial.print(currentVoltage, 1);
        Serial.print(F(" ADC: "));
        Serial.println(newSample);
        lastDebug = millis();
    }
#endif
}

float Voltage::getVoltage() {
    return currentVoltage;
}

uint16_t Voltage::getRawADC() {
    return samples[(sampleIndex == 0) ? VOLTAGE_SAMPLES - 1 : sampleIndex - 1];
}

bool Voltage::isArcOk() {
    return currentVoltage >= ARC_OK_VOLTAGE;
}

bool Voltage::isArcLost() {
    return currentVoltage < LOST_ARC_VOLTAGE;
}

void Voltage::startSampling() {
    sampling = true;
    smartSampleCount = 0;
    smartSampleSum = 0;
    samplingStartTime = millis();
}

void Voltage::stopSampling() {
    sampling = false;
}

float Voltage::getSampledVoltage() {
    if (smartSampleCount == 0) {
        return currentVoltage;
    }
    return smartSampleSum / smartSampleCount;
}

bool Voltage::isSamplingComplete() {
    return smartSampleCount >= settings.data.sampleSize;
}

float Voltage::adcToVoltage(uint16_t adc) {
    // Convert ADC reading to actual arc voltage
    // ADC value -> reference voltage -> through voltage divider
    float adcVoltage = (float)adc * ADC_REFERENCE / ADC_RESOLUTION;
    return adcVoltage * VOLTAGE_DIVIDER;
}
