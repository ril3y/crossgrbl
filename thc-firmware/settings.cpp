/*
 * settings.cpp - EEPROM Settings Implementation
 */

#include "settings.h"
#include <EEPROM.h>

// Global instance
Settings settings;

Settings::Settings() {
    reset();  // Initialize with defaults
}

void Settings::begin() {
    if (!load()) {
        // EEPROM not initialized or corrupted, use defaults
        reset();
        save();
    }
    validate();
}

bool Settings::load() {
    // Check magic bytes
    uint16_t magic;
    EEPROM.get(EEPROM_ADDR_MAGIC, magic);

    if (magic != MAGIC) {
        return false;  // EEPROM not initialized
    }

    // Load settings data
    EEPROM.get(EEPROM_ADDR_DATA, data);

    // Validate loaded data
    if (!isValid()) {
        return false;
    }

    return true;
}

void Settings::save() {
    validate();  // Ensure values are in range before saving

    // Write magic bytes
    EEPROM.put(EEPROM_ADDR_MAGIC, MAGIC);

    // Write settings data
    EEPROM.put(EEPROM_ADDR_DATA, data);
}

void Settings::reset() {
    data.nominalVoltage = DEFAULT_NOMINAL_VOLTAGE;
    data.voltageTolerance = DEFAULT_VOLTAGE_TOLERANCE;
    data.speedCutoff = DEFAULT_SPEED_CUTOFF;
    data.sampleSize = DEFAULT_SAMPLE_SIZE;
    data.sampleDelay = DEFAULT_SAMPLE_DELAY;
    data.overrideSpeed = DEFAULT_OVERRIDE_SPEED;
    data.smartMode = false;
}

bool Settings::isValid() {
    // Check for NaN or extreme values
    if (isnan(data.nominalVoltage) || data.nominalVoltage < 0 || data.nominalVoltage > 500) {
        return false;
    }
    if (isnan(data.voltageTolerance) || data.voltageTolerance < 0 || data.voltageTolerance > 50) {
        return false;
    }
    if (data.speedCutoff > 100) {
        return false;
    }
    if (data.sampleSize == 0 || data.sampleSize > 100) {
        return false;
    }
    if (data.sampleDelay > 10000) {
        return false;
    }
    if (data.overrideSpeed == 0 || data.overrideSpeed > 100) {
        return false;
    }
    return true;
}

void Settings::validate() {
    // Clamp values to valid ranges
    if (isnan(data.nominalVoltage) || data.nominalVoltage < 0) {
        data.nominalVoltage = DEFAULT_NOMINAL_VOLTAGE;
    }
    if (data.nominalVoltage > 500) {
        data.nominalVoltage = 500;
    }

    if (isnan(data.voltageTolerance) || data.voltageTolerance < 0) {
        data.voltageTolerance = DEFAULT_VOLTAGE_TOLERANCE;
    }
    if (data.voltageTolerance > 50) {
        data.voltageTolerance = 50;
    }

    if (data.speedCutoff > 100) {
        data.speedCutoff = DEFAULT_SPEED_CUTOFF;
    }

    if (data.sampleSize == 0 || data.sampleSize > 100) {
        data.sampleSize = DEFAULT_SAMPLE_SIZE;
    }

    if (data.sampleDelay > 10000) {
        data.sampleDelay = DEFAULT_SAMPLE_DELAY;
    }

    if (data.overrideSpeed == 0 || data.overrideSpeed > 100) {
        data.overrideSpeed = DEFAULT_OVERRIDE_SPEED;
    }
}
