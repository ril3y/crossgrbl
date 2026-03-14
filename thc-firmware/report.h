/*
 * report.h - Status and Settings Report Formatting
 *
 * Formats responses for FireControl protocol compatibility.
 */

#ifndef REPORT_H
#define REPORT_H

#include <Arduino.h>
#include "config.h"
#include "settings.h"

class Report {
public:
    Report();

    // Send status response
    // Format: <state>|<liveV>|<nominalV>|<tolerance>|<speed>|<progSpeed>|<cutoff>|<bitflags>|<error>
    void sendStatus(thc_state_t state, float liveVoltage, uint8_t currentSpeed,
                    uint8_t programmedSpeed, uint8_t bitflags, uint8_t errorCode);

    // Send all settings
    // Format: VS: value, VT: value, etc.
    void sendSettings();

private:
    // State to string for status
    uint8_t stateToCode(thc_state_t state);
};

// Global instance
extern Report report;

#endif // REPORT_H
