/*
 * anti_dive.h - Anti-Dive Speed Detection
 *
 * Monitors X/Y step pulses to detect travel speed and freeze
 * THC when speed drops below cutoff (prevents corner dive).
 */

#ifndef ANTI_DIVE_H
#define ANTI_DIVE_H

#include <Arduino.h>
#include "config.h"

class AntiDive {
public:
    AntiDive();

    // Initialize interrupts
    void begin();

    // Update speed calculation (call from main loop)
    void update();

    // Check if speed is below cutoff
    bool isBelowCutoff();

    // Get current speed as percentage of max observed
    uint8_t getCurrentSpeedPercent();

    // Get raw pulse counts (for debugging)
    uint32_t getXCount();
    uint32_t getYCount();

    // Reset speed tracking (call at cut start)
    void reset();

    // Calibrate max speed (call during fast travel)
    void calibrateMax();

    // ISR callbacks (must be public for ISR access)
    static void xStepISR();
    static void yStepISR();

private:
    // Pulse counts (volatile for ISR access)
    static volatile uint32_t xPulseCount;
    static volatile uint32_t yPulseCount;

    // Speed calculation
    uint32_t lastCalcTime;
    uint32_t lastXCount;
    uint32_t lastYCount;

    // Speed history for averaging
    uint16_t speedHistory[SPEED_HISTORY_SIZE];
    uint8_t historyIndex;

    // Calibration
    uint32_t maxPulsesPerInterval;  // Maximum observed pulse rate
    bool calibrated;

    // Calculate combined XY speed from pulse counts
    uint32_t calculateSpeed();
};

// Global instance
extern AntiDive antiDive;

#endif // ANTI_DIVE_H
