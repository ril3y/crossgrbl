/*
 * report.cpp - Status and Settings Report Implementation
 */

#include "report.h"
#include "protocol.h"

// Global instance
Report report;

Report::Report() {
}

void Report::sendStatus(thc_state_t state, float liveVoltage, uint8_t currentSpeed,
                        uint8_t programmedSpeed, uint8_t bitflags, uint8_t errorCode) {
    // Format: <state>|<liveV>|<nominalV>|<tolerance>|<speed>|<progSpeed>|<cutoff>|<bitflags>|<error>

    Serial.print(stateToCode(state));
    Serial.print('|');
    Serial.print(liveVoltage, 1);
    Serial.print('|');
    Serial.print(settings.data.nominalVoltage, 1);
    Serial.print('|');
    Serial.print(settings.data.voltageTolerance, 1);
    Serial.print('|');
    Serial.print(currentSpeed);
    Serial.print('|');
    Serial.print(programmedSpeed);
    Serial.print('|');
    Serial.print(settings.data.speedCutoff);
    Serial.print('|');
    Serial.print(bitflags);
    Serial.print('|');
    Serial.println(errorCode);

    protocol.sendOk();
}

void Report::sendSettings() {
    // Send all settings in FireControl expected format
    protocol.sendValue("VS", (int)settings.data.sampleSize);
    protocol.sendValue("VT", settings.data.voltageTolerance, 1);
    protocol.sendValue("VN", settings.data.nominalVoltage, 1);
    protocol.sendValue("PT", (int)settings.data.speedCutoff);
    protocol.sendValue("VA", (int)settings.data.sampleDelay);
    protocol.sendValue("SO", (int)settings.data.overrideSpeed);
    protocol.sendValue("SM", settings.data.smartMode ? 1 : 0);

    protocol.sendOk();
}

uint8_t Report::stateToCode(thc_state_t state) {
    // Map internal state to protocol state codes
    switch (state) {
        case STATE_IDLE:
            return 0;
        case STATE_ENABLED:
            return 1;
        case STATE_TORCH_ON:
            return 2;
        case STATE_ARC_OK:
            return 3;
        case STATE_FROZEN:
            return 4;
        case STATE_ERROR:
            return 9;
        default:
            return 0;
    }
}
