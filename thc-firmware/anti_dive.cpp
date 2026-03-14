/*
 * anti_dive.cpp - Anti-Dive Speed Detection Implementation
 */

#include "anti_dive.h"
#include "settings.h"

// Global instance
AntiDive antiDive;

// Static volatile variables for ISR access
volatile uint32_t AntiDive::xPulseCount = 0;
volatile uint32_t AntiDive::yPulseCount = 0;

AntiDive::AntiDive() {
    lastCalcTime = 0;
    lastXCount = 0;
    lastYCount = 0;
    historyIndex = 0;
    maxPulsesPerInterval = 1000;  // Default, will be calibrated
    calibrated = false;

    memset(speedHistory, 0, sizeof(speedHistory));
}

void AntiDive::begin() {
    // Configure step input pins with pullup
    pinMode(PIN_X_STEP, INPUT_PULLUP);
    pinMode(PIN_Y_STEP, INPUT_PULLUP);

    // Attach interrupts for step pulse counting
    // Using RISING edge to count step pulses
    attachInterrupt(digitalPinToInterrupt(PIN_X_STEP), xStepISR, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_Y_STEP), yStepISR, RISING);

    reset();
}

void AntiDive::reset() {
    // Reset counters
    noInterrupts();
    xPulseCount = 0;
    yPulseCount = 0;
    interrupts();

    lastXCount = 0;
    lastYCount = 0;
    lastCalcTime = millis();
    historyIndex = 0;

    memset(speedHistory, 0, sizeof(speedHistory));
}

void AntiDive::update() {
    uint32_t now = millis();

    // Calculate speed at configured interval
    if (now - lastCalcTime < SPEED_CALC_INTERVAL_MS) {
        return;
    }

    // Get current pulse counts atomically
    noInterrupts();
    uint32_t currentX = xPulseCount;
    uint32_t currentY = yPulseCount;
    interrupts();

    // Calculate pulses since last check
    uint32_t deltaX = currentX - lastXCount;
    uint32_t deltaY = currentY - lastYCount;

    lastXCount = currentX;
    lastYCount = currentY;
    lastCalcTime = now;

    // Combined speed (Pythagorean would be more accurate but this is simpler)
    uint32_t speed = deltaX + deltaY;

    // Update max if calibrating
    if (calibrated == false && speed > maxPulsesPerInterval) {
        maxPulsesPerInterval = speed;
    }

    // Add to history
    speedHistory[historyIndex] = (uint16_t)min(speed, 65535UL);
    historyIndex = (historyIndex + 1) % SPEED_HISTORY_SIZE;

#ifdef DEBUG_SPEED
    static uint32_t lastDebug = 0;
    if (millis() - lastDebug > 500) {
        Serial.print(F("Speed: "));
        Serial.print(speed);
        Serial.print(F(" max: "));
        Serial.print(maxPulsesPerInterval);
        Serial.print(F(" %: "));
        Serial.println(getCurrentSpeedPercent());
        lastDebug = millis();
    }
#endif
}

uint32_t AntiDive::calculateSpeed() {
    // Calculate average from history
    uint32_t sum = 0;
    for (uint8_t i = 0; i < SPEED_HISTORY_SIZE; i++) {
        sum += speedHistory[i];
    }
    return sum / SPEED_HISTORY_SIZE;
}

bool AntiDive::isBelowCutoff() {
    // If not cutting (no pulses), don't report below cutoff
    uint32_t avgSpeed = calculateSpeed();
    if (avgSpeed == 0 && maxPulsesPerInterval == 0) {
        return false;
    }

    uint8_t percent = getCurrentSpeedPercent();
    return percent < settings.data.speedCutoff;
}

uint8_t AntiDive::getCurrentSpeedPercent() {
    if (maxPulsesPerInterval == 0) {
        return 100;  // No reference yet, assume full speed
    }

    uint32_t avgSpeed = calculateSpeed();
    uint32_t percent = (avgSpeed * 100) / maxPulsesPerInterval;

    // Cap at 100%
    if (percent > 100) {
        percent = 100;
    }

    return (uint8_t)percent;
}

uint32_t AntiDive::getXCount() {
    noInterrupts();
    uint32_t count = xPulseCount;
    interrupts();
    return count;
}

uint32_t AntiDive::getYCount() {
    noInterrupts();
    uint32_t count = yPulseCount;
    interrupts();
    return count;
}

void AntiDive::calibrateMax() {
    // Reset max and allow recalibration
    maxPulsesPerInterval = 0;
    calibrated = false;
}

// ISR implementations
void AntiDive::xStepISR() {
    xPulseCount++;
}

void AntiDive::yStepISR() {
    yPulseCount++;
}
