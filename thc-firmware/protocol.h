/*
 * protocol.h - Serial Command Protocol Handler
 *
 * Parses commands from FireControl and dispatches to appropriate handlers.
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>
#include "config.h"

// Command types
typedef enum {
    CMD_NONE = 0,
    CMD_STATUS,         // ?  - Request status
    CMD_SETTINGS,       // $  - Request settings
    CMD_ENABLE,         // ON - Enable THC
    CMD_DISABLE,        // OF - Disable THC
    CMD_TORCH_ON,       // TO - Torch fire signal
    CMD_TORCH_OFF,      // TF - Torch off signal
    CMD_SET_NOMINAL,    // VN<value> - Set nominal voltage
    CMD_SET_TOLERANCE,  // VT<value> - Set voltage tolerance
    CMD_SET_CUTOFF,     // PT<value> - Set speed cutoff %
    CMD_SET_SAMPLE,     // VS<value> - Set sample size
    CMD_SET_DELAY,      // VA<value> - Set sample delay (alternative name)
    CMD_CLEAR_ERROR,    // CE - Clear error
    CMD_OVERRIDE_UP,    // ZU - Override up
    CMD_OVERRIDE_DOWN,  // ZD - Override down
    CMD_OVERRIDE_STOP,  // ZS - Override stop
    CMD_SET_OVERRIDE_SPEED, // SO<value> - Set override speed
    CMD_SMART_ON,       // SM - Smart mode on
    CMD_SMART_OFF,      // SF - Smart mode off
    CMD_SAVE,           // SV - Save settings
    CMD_RESET,          // RS - Reset to defaults
    CMD_UNKNOWN
} cmd_type_t;

// Parsed command structure
typedef struct {
    cmd_type_t type;
    float value;        // For commands with numeric parameter
    bool has_value;
} command_t;

class Protocol {
public:
    Protocol();

    // Initialize serial communication
    void begin();

    // Poll for incoming commands (call from main loop)
    // Returns true if a complete command was received
    bool poll();

    // Get the last parsed command
    command_t getCommand();

    // Send response to FireControl
    void sendOk();
    void sendError(const char* msg);
    void sendLine(const char* line);
    void sendValue(const char* name, float value, int decimals = 1);
    void sendValue(const char* name, int value);

    // Send startup banner
    void sendBanner();

private:
    char buffer[CMD_BUFFER_SIZE];
    uint8_t bufferIndex;
    command_t lastCommand;

    // Parse buffer contents into command
    void parseBuffer();

    // Parse numeric value from buffer starting at position
    float parseValue(uint8_t startPos);
};

// Global instance
extern Protocol protocol;

#endif // PROTOCOL_H
