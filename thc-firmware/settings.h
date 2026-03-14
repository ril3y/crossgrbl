/*
 * settings.h - EEPROM Settings Storage
 *
 * Persists THC configuration parameters to EEPROM.
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include "config.h"

// Settings data structure
typedef struct {
    float nominalVoltage;    // Target arc voltage (V)
    float voltageTolerance;  // Deadband tolerance (V)
    uint8_t speedCutoff;     // Speed cutoff percentage
    uint8_t sampleSize;      // Smart voltage sample count
    uint16_t sampleDelay;    // Sample delay (ms)
    uint8_t overrideSpeed;   // Override mode speed (steps/sec)
    bool smartMode;          // Smart voltage sampling enabled
} settings_t;

class Settings {
public:
    Settings();

    // Initialize - load from EEPROM or set defaults
    void begin();

    // Load settings from EEPROM
    bool load();

    // Save current settings to EEPROM
    void save();

    // Reset to factory defaults
    void reset();

    // Current settings (public for direct access)
    settings_t data;

    // Validation
    bool isValid();
    void validate();

private:
    static const uint16_t MAGIC = 0x5448;  // "TH" for THC
};

// Global instance
extern Settings settings;

#endif // SETTINGS_H
