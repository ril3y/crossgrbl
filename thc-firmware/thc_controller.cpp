/*
 * thc_controller.cpp - THC State Machine Implementation
 */

#include "thc_controller.h"
#include "voltage.h"
#include "motion.h"
#include "anti_dive.h"
#include "settings.h"

// Global instance
THCController thc;

THCController::THCController() {
    state = STATE_IDLE;
    errorCode = ERROR_NONE;
    bitflags = 0;
    overrideDir = OVERRIDE_NONE;
    overrideActive = false;
    torchOnTime = 0;
    arcOkTime = 0;
    arcLostTime = 0;
    arcWasOk = false;
    smartModeActive = false;
    nominalCaptured = false;
}

void THCController::begin() {
    state = STATE_IDLE;
    errorCode = ERROR_NONE;
    bitflags = 0;
    smartModeActive = settings.data.smartMode;
}

void THCController::update() {
    // State machine dispatch
    switch (state) {
        case STATE_IDLE:
            handleIdle();
            break;
        case STATE_ENABLED:
            handleEnabled();
            break;
        case STATE_TORCH_ON:
            handleTorchOn();
            break;
        case STATE_ARC_OK:
            handleArcOk();
            break;
        case STATE_FROZEN:
            handleFrozen();
            break;
        case STATE_ERROR:
            handleError();
            break;
    }

    // Handle override motion in any state except error
    if (overrideActive && state != STATE_ERROR) {
        if (overrideDir == OVERRIDE_UP) {
            motion.stepUp();
            updateBitflags(1);
        } else if (overrideDir == OVERRIDE_DOWN) {
            motion.stepDown();
            updateBitflags(-1);
        }
    }

#ifdef DEBUG_STATE
    static thc_state_t lastState = STATE_IDLE;
    if (state != lastState) {
        Serial.print(F("State: "));
        Serial.println(state);
        lastState = state;
    }
#endif
}

void THCController::handleIdle() {
    // Idle state - waiting for enable command
    // Z motion disabled, no voltage tracking
    bitflags = 0;
}

void THCController::handleEnabled() {
    // THC enabled but torch not firing
    // Ready to respond to torch on command
    bitflags = FLAG_ACTIVE;
}

void THCController::handleTorchOn() {
    // Torch is on, waiting for arc to establish
    uint32_t now = millis();

    // Wait for arc OK delay before checking
    if (now - torchOnTime < ARC_OK_DELAY_MS) {
        return;
    }

    // Check for arc establishment
    if (voltage.isArcOk()) {
        arcOkTime = now;
        arcWasOk = true;

        // Start smart voltage sampling if enabled
        if (smartModeActive) {
            voltage.startSampling();
            nominalCaptured = false;
        }

        setState(STATE_ARC_OK);
    }
}

void THCController::handleArcOk() {
    uint32_t now = millis();

    // Smart voltage sampling
    if (smartModeActive && !nominalCaptured && voltage.isSamplingComplete()) {
        // Capture the sampled voltage as nominal
        settings.data.nominalVoltage = voltage.getSampledVoltage();
        voltage.stopSampling();
        nominalCaptured = true;
    }

    // Check for arc loss
    if (voltage.isArcLost()) {
        if (arcLostTime == 0) {
            arcLostTime = now;
        } else if (now - arcLostTime > LOST_ARC_DELAY_MS) {
            setError(ERROR_LOST_ARC);
            return;
        }
    } else {
        arcLostTime = 0;
    }

    // Check anti-dive speed cutoff
    if (antiDive.isBelowCutoff()) {
        setState(STATE_FROZEN);
        return;
    }

    // Run voltage tracking control loop
    runControlLoop();
}

void THCController::handleFrozen() {
    // THC frozen due to low speed - don't adjust height

    // Check if speed recovered
    if (!antiDive.isBelowCutoff()) {
        setState(STATE_ARC_OK);
        return;
    }

    // Still monitor for arc loss
    if (voltage.isArcLost()) {
        if (arcLostTime == 0) {
            arcLostTime = millis();
        } else if (millis() - arcLostTime > LOST_ARC_DELAY_MS) {
            setError(ERROR_LOST_ARC);
            return;
        }
    } else {
        arcLostTime = 0;
    }

    // Update flags to show frozen state
    bitflags = FLAG_ACTIVE | FLAG_ARC_OK;
    if (smartModeActive) {
        bitflags |= FLAG_SMART_MODE;
    }
}

void THCController::handleError() {
    // Error state - motion stopped, waiting for clear
    motion.stop();
    bitflags = 0;
}

void THCController::runControlLoop() {
    float currentV = voltage.getVoltage();
    float nominalV = settings.data.nominalVoltage;
    float tolerance = settings.data.voltageTolerance;

    float error = currentV - nominalV;
    int8_t direction = 0;

    if (error > tolerance) {
        // Voltage too high - torch too close, move UP
        motion.stepUp();
        direction = 1;
    } else if (error < -tolerance) {
        // Voltage too low - torch too far, move DOWN
        motion.stepDown();
        direction = -1;
    } else {
        // Within tolerance - at target
        motion.stop();
        direction = 0;
    }

    updateBitflags(direction);
}

void THCController::updateBitflags(int8_t direction) {
    bitflags = FLAG_ACTIVE | FLAG_ARC_OK;

    if (smartModeActive) {
        bitflags |= FLAG_SMART_MODE;
    }

    if (overrideActive) {
        bitflags |= FLAG_OVERRIDE;
    }

    if (direction > 0) {
        bitflags |= FLAG_MOVING_UP;
    } else if (direction < 0) {
        bitflags |= FLAG_MOVING_DOWN;
    } else {
        bitflags |= FLAG_AT_TARGET;
    }
}

void THCController::setState(thc_state_t newState) {
    state = newState;

#ifdef DEBUG_STATE
    Serial.print(F("-> State: "));
    Serial.println(newState);
#endif
}

void THCController::setError(uint8_t code) {
    errorCode = code;
    setState(STATE_ERROR);
    motion.stop();
    voltage.stopSampling();

#ifdef DEBUG_STATE
    Serial.print(F("ERROR: "));
    Serial.println(code);
#endif
}

// Public control methods

void THCController::enable() {
    if (state == STATE_IDLE || state == STATE_ERROR) {
        errorCode = ERROR_NONE;
        setState(STATE_ENABLED);
    }
}

void THCController::disable() {
    motion.stop();
    voltage.stopSampling();
    overrideActive = false;
    overrideDir = OVERRIDE_NONE;
    setState(STATE_IDLE);
}

void THCController::torchOn() {
    if (state == STATE_ENABLED) {
        torchOnTime = millis();
        arcLostTime = 0;
        arcWasOk = false;
        nominalCaptured = false;
        setState(STATE_TORCH_ON);
    }
}

void THCController::torchOff() {
    motion.stop();
    voltage.stopSampling();

    if (state == STATE_TORCH_ON || state == STATE_ARC_OK || state == STATE_FROZEN) {
        setState(STATE_ENABLED);
    }
}

void THCController::clearError() {
    if (state == STATE_ERROR) {
        errorCode = ERROR_NONE;
        setState(STATE_IDLE);
    }
}

void THCController::overrideUp() {
    overrideActive = true;
    overrideDir = OVERRIDE_UP;
    bitflags |= FLAG_OVERRIDE;
}

void THCController::overrideDown() {
    overrideActive = true;
    overrideDir = OVERRIDE_DOWN;
    bitflags |= FLAG_OVERRIDE;
}

void THCController::overrideStop() {
    overrideActive = false;
    overrideDir = OVERRIDE_NONE;
    motion.stop();
    bitflags &= ~FLAG_OVERRIDE;
}

void THCController::setSmartMode(bool enabled) {
    smartModeActive = enabled;
    settings.data.smartMode = enabled;
}

// Status getters

thc_state_t THCController::getState() {
    return state;
}

uint8_t THCController::getBitflags() {
    return bitflags;
}

uint8_t THCController::getErrorCode() {
    return errorCode;
}

uint8_t THCController::getCurrentSpeed() {
    return antiDive.getCurrentSpeedPercent();
}

uint8_t THCController::getProgrammedSpeed() {
    return 100;  // TODO: Get from GRBL if connected
}
