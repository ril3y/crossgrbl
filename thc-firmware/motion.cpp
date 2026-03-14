/*
 * motion.cpp - Z Axis Motion Implementation
 */

#include "motion.h"
#include "settings.h"

// Global instance
Motion motion;

Motion::Motion() {
    enabled = false;
    direction = 0;
    stepping = false;
    lastStepTime = 0;
    stepInterval = STEP_INTERVAL_US;
}

void Motion::begin() {
    // Configure pins
    pinMode(PIN_Z_STEP, OUTPUT);
    pinMode(PIN_Z_DIR, OUTPUT);
    pinMode(PIN_Z_ENABLE, OUTPUT);

    // Initialize outputs
    digitalWrite(PIN_Z_STEP, LOW);
    digitalWrite(PIN_Z_DIR, Z_DIR_UP);
    digitalWrite(PIN_Z_ENABLE, Z_DISABLED);

    enabled = false;
    direction = 0;
}

void Motion::stepUp() {
    if (!enabled) {
        enable();
    }
    direction = 1;
    digitalWrite(PIN_Z_DIR, Z_DIR_UP);
}

void Motion::stepDown() {
    if (!enabled) {
        enable();
    }
    direction = -1;
    digitalWrite(PIN_Z_DIR, Z_DIR_DOWN);
}

void Motion::stop() {
    direction = 0;
    stepping = false;
}

void Motion::update() {
    // If not moving, nothing to do
    if (direction == 0) {
        return;
    }

    uint32_t now = micros();

    // Check if it's time for next step
    if (now - lastStepTime >= stepInterval) {
        pulse();
        lastStepTime = now;
    }

#ifdef DEBUG_MOTION
    static uint32_t lastDebug = 0;
    if (millis() - lastDebug > 500) {
        Serial.print(F("Motion: dir="));
        Serial.print(direction);
        Serial.print(F(" interval="));
        Serial.println(stepInterval);
        lastDebug = millis();
    }
#endif
}

void Motion::pulse() {
    // Generate step pulse
    digitalWrite(PIN_Z_STEP, HIGH);
    delayMicroseconds(STEP_PULSE_US);
    digitalWrite(PIN_Z_STEP, LOW);
}

bool Motion::isMoving() {
    return direction != 0;
}

int8_t Motion::getDirection() {
    return direction;
}

void Motion::enable() {
    digitalWrite(PIN_Z_ENABLE, Z_ENABLED);
    enabled = true;

    // Calculate step interval from override speed setting
    // Override speed is in steps/second
    uint8_t speed = settings.data.overrideSpeed;
    if (speed == 0) speed = 1;  // Prevent division by zero

    // Convert steps/sec to microseconds/step
    stepInterval = 1000000UL / speed;

    // Clamp to minimum step interval
    if (stepInterval < STEP_INTERVAL_US) {
        stepInterval = STEP_INTERVAL_US;
    }
}

void Motion::disable() {
    digitalWrite(PIN_Z_ENABLE, Z_DISABLED);
    enabled = false;
    direction = 0;
}
