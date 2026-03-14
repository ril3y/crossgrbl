/*
 * thc_controller.h - THC State Machine Controller
 *
 * Main control logic for Torch Height Controller operation.
 */

#ifndef THC_CONTROLLER_H
#define THC_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

// Override direction
typedef enum {
    OVERRIDE_NONE = 0,
    OVERRIDE_UP,
    OVERRIDE_DOWN
} override_dir_t;

class THCController {
public:
    THCController();

    // Initialize controller
    void begin();

    // Main update loop (call frequently)
    void update();

    // State control
    void enable();
    void disable();
    void torchOn();
    void torchOff();
    void clearError();

    // Override control
    void overrideUp();
    void overrideDown();
    void overrideStop();

    // Smart mode
    void setSmartMode(bool enabled);

    // Status getters
    thc_state_t getState();
    uint8_t getBitflags();
    uint8_t getErrorCode();
    uint8_t getCurrentSpeed();
    uint8_t getProgrammedSpeed();

private:
    thc_state_t state;
    uint8_t errorCode;
    uint8_t bitflags;

    // Override state
    override_dir_t overrideDir;
    bool overrideActive;

    // Arc detection timing
    uint32_t torchOnTime;
    uint32_t arcOkTime;
    uint32_t arcLostTime;
    bool arcWasOk;

    // Smart voltage
    bool smartModeActive;
    bool nominalCaptured;

    // State handlers
    void handleIdle();
    void handleEnabled();
    void handleTorchOn();
    void handleArcOk();
    void handleFrozen();
    void handleError();

    // Control loop
    void runControlLoop();

    // Update bitflags based on current state
    void updateBitflags(int8_t direction);

    // Transition to new state
    void setState(thc_state_t newState);

    // Set error and transition to error state
    void setError(uint8_t code);
};

// Global instance
extern THCController thc;

#endif // THC_CONTROLLER_H
