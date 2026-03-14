/*
 * THC Firmware - FireControl Compatible Torch Height Controller
 *
 * Open-source replacement for Langmuir LS-THC module.
 * Compatible with FireControl software via CH340 USB serial.
 *
 * Hardware: Arduino Nano clone with CH340 (VID:1A86 PID:7523)
 *
 * License: MIT
 */

#include "config.h"
#include "protocol.h"
#include "settings.h"
#include "voltage.h"
#include "motion.h"
#include "anti_dive.h"
#include "thc_controller.h"
#include "report.h"

// Status LED blink timing
uint32_t lastBlinkTime = 0;
bool ledState = false;

void setup() {
    // Initialize status LED
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, HIGH);  // LED on during init

    // Initialize all modules
    settings.begin();     // Load settings from EEPROM
    protocol.begin();     // Start serial communication (sends banner)
    voltage.begin();      // Initialize ADC
    motion.begin();       // Initialize motor pins
    antiDive.begin();     // Initialize speed detection interrupts
    thc.begin();          // Initialize state machine

    // Init complete
    digitalWrite(PIN_STATUS_LED, LOW);
}

void loop() {
    // Update all modules
    voltage.update();
    motion.update();
    antiDive.update();
    thc.update();

    // Process serial commands
    if (protocol.poll()) {
        handleCommand(protocol.getCommand());
    }

    // Status LED blink based on state
    updateStatusLED();
}

void handleCommand(command_t cmd) {
    switch (cmd.type) {
        case CMD_STATUS:
            report.sendStatus(
                thc.getState(),
                voltage.getVoltage(),
                thc.getCurrentSpeed(),
                thc.getProgrammedSpeed(),
                thc.getBitflags(),
                thc.getErrorCode()
            );
            break;

        case CMD_SETTINGS:
            report.sendSettings();
            break;

        case CMD_ENABLE:
            thc.enable();
            protocol.sendOk();
            break;

        case CMD_DISABLE:
            thc.disable();
            protocol.sendOk();
            break;

        case CMD_TORCH_ON:
            thc.torchOn();
            protocol.sendOk();
            break;

        case CMD_TORCH_OFF:
            thc.torchOff();
            protocol.sendOk();
            break;

        case CMD_SET_NOMINAL:
            if (cmd.has_value) {
                settings.data.nominalVoltage = cmd.value;
            }
            protocol.sendOk();
            break;

        case CMD_SET_TOLERANCE:
            if (cmd.has_value) {
                settings.data.voltageTolerance = cmd.value;
            }
            protocol.sendOk();
            break;

        case CMD_SET_CUTOFF:
            if (cmd.has_value) {
                settings.data.speedCutoff = (uint8_t)cmd.value;
            }
            protocol.sendOk();
            break;

        case CMD_SET_SAMPLE:
            if (cmd.has_value) {
                settings.data.sampleSize = (uint8_t)cmd.value;
            }
            protocol.sendOk();
            break;

        case CMD_SET_DELAY:
            if (cmd.has_value) {
                settings.data.sampleDelay = (uint16_t)cmd.value;
            }
            protocol.sendOk();
            break;

        case CMD_SET_OVERRIDE_SPEED:
            if (cmd.has_value) {
                settings.data.overrideSpeed = (uint8_t)cmd.value;
            }
            protocol.sendOk();
            break;

        case CMD_CLEAR_ERROR:
            thc.clearError();
            protocol.sendOk();
            break;

        case CMD_OVERRIDE_UP:
            thc.overrideUp();
            protocol.sendOk();
            break;

        case CMD_OVERRIDE_DOWN:
            thc.overrideDown();
            protocol.sendOk();
            break;

        case CMD_OVERRIDE_STOP:
            thc.overrideStop();
            protocol.sendOk();
            break;

        case CMD_SMART_ON:
            thc.setSmartMode(true);
            protocol.sendOk();
            break;

        case CMD_SMART_OFF:
            thc.setSmartMode(false);
            protocol.sendOk();
            break;

        case CMD_SAVE:
            settings.save();
            protocol.sendOk();
            break;

        case CMD_RESET:
            settings.reset();
            settings.save();
            protocol.sendOk();
            break;

        case CMD_UNKNOWN:
            protocol.sendError("unknown command");
            break;

        case CMD_NONE:
        default:
            break;
    }
}

void updateStatusLED() {
    uint32_t now = millis();
    uint32_t blinkInterval;

    // Blink rate depends on state
    switch (thc.getState()) {
        case STATE_IDLE:
            blinkInterval = 2000;  // Slow blink
            break;
        case STATE_ENABLED:
            blinkInterval = 1000;  // Medium blink
            break;
        case STATE_TORCH_ON:
            blinkInterval = 250;   // Fast blink
            break;
        case STATE_ARC_OK:
            blinkInterval = 0;     // Solid on
            digitalWrite(PIN_STATUS_LED, HIGH);
            return;
        case STATE_FROZEN:
            blinkInterval = 100;   // Very fast blink
            break;
        case STATE_ERROR:
            blinkInterval = 150;   // Error pattern
            break;
        default:
            blinkInterval = 1000;
    }

    if (blinkInterval > 0 && now - lastBlinkTime >= blinkInterval) {
        lastBlinkTime = now;
        ledState = !ledState;
        digitalWrite(PIN_STATUS_LED, ledState);
    }
}
