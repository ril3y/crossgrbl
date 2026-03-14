/*
 * motion.h - Z Axis Motion Control
 *
 * Generates step pulses for Z stepper motor.
 */

#ifndef MOTION_H
#define MOTION_H

#include <Arduino.h>
#include "config.h"

class Motion {
public:
    Motion();

    // Initialize motor pins
    void begin();

    // Step generation (call from main loop or control loop)
    void stepUp();
    void stepDown();
    void stop();

    // Update - generate steps at configured rate
    void update();

    // Status
    bool isMoving();
    int8_t getDirection();  // -1 down, 0 stopped, 1 up

    // Enable/disable motor driver
    void enable();
    void disable();

private:
    bool enabled;
    int8_t direction;  // -1, 0, +1
    bool stepping;     // Currently generating a step
    uint32_t lastStepTime;
    uint32_t stepInterval;  // Microseconds between steps

    // Generate single step pulse
    void pulse();
};

// Global instance
extern Motion motion;

#endif // MOTION_H
