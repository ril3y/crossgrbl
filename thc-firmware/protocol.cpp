/*
 * protocol.cpp - Serial Command Protocol Implementation
 */

#include "protocol.h"

// Global instance
Protocol protocol;

Protocol::Protocol() {
    bufferIndex = 0;
    memset(buffer, 0, CMD_BUFFER_SIZE);
    lastCommand.type = CMD_NONE;
    lastCommand.value = 0;
    lastCommand.has_value = false;
}

void Protocol::begin() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial) {
        ; // Wait for serial port to connect (needed for USB)
    }
    sendBanner();
}

void Protocol::sendBanner() {
    Serial.println(THC_VERSION);
}

bool Protocol::poll() {
    while (Serial.available()) {
        char c = Serial.read();

        // Handle line endings
        if (c == '\n' || c == '\r') {
            if (bufferIndex > 0) {
                buffer[bufferIndex] = '\0';
                parseBuffer();
                bufferIndex = 0;
                return true;
            }
        }
        // Handle single-character commands immediately
        else if (c == '?' || c == '$') {
            buffer[0] = c;
            buffer[1] = '\0';
            bufferIndex = 0;
            parseBuffer();
            return true;
        }
        // Add to buffer
        else if (bufferIndex < CMD_BUFFER_SIZE - 1) {
            buffer[bufferIndex++] = c;
        }
    }
    return false;
}

void Protocol::parseBuffer() {
    lastCommand.type = CMD_NONE;
    lastCommand.value = 0;
    lastCommand.has_value = false;

    if (bufferIndex == 0 && buffer[0] == '\0') {
        return;
    }

    // Single character commands
    if (buffer[0] == '?' && buffer[1] == '\0') {
        lastCommand.type = CMD_STATUS;
        return;
    }
    if (buffer[0] == '$' && buffer[1] == '\0') {
        lastCommand.type = CMD_SETTINGS;
        return;
    }

    // Two character commands
    if (buffer[0] == 'O' && buffer[1] == 'N') {
        lastCommand.type = CMD_ENABLE;
        return;
    }
    if (buffer[0] == 'O' && buffer[1] == 'F') {
        lastCommand.type = CMD_DISABLE;
        return;
    }
    if (buffer[0] == 'T' && buffer[1] == 'O') {
        lastCommand.type = CMD_TORCH_ON;
        return;
    }
    if (buffer[0] == 'T' && buffer[1] == 'F') {
        lastCommand.type = CMD_TORCH_OFF;
        return;
    }
    if (buffer[0] == 'C' && buffer[1] == 'E') {
        lastCommand.type = CMD_CLEAR_ERROR;
        return;
    }
    if (buffer[0] == 'Z' && buffer[1] == 'U') {
        lastCommand.type = CMD_OVERRIDE_UP;
        return;
    }
    if (buffer[0] == 'Z' && buffer[1] == 'D') {
        lastCommand.type = CMD_OVERRIDE_DOWN;
        return;
    }
    if (buffer[0] == 'Z' && buffer[1] == 'S') {
        lastCommand.type = CMD_OVERRIDE_STOP;
        return;
    }
    if (buffer[0] == 'S' && buffer[1] == 'M') {
        lastCommand.type = CMD_SMART_ON;
        return;
    }
    if (buffer[0] == 'S' && buffer[1] == 'F') {
        lastCommand.type = CMD_SMART_OFF;
        return;
    }
    if (buffer[0] == 'S' && buffer[1] == 'V') {
        lastCommand.type = CMD_SAVE;
        return;
    }
    if (buffer[0] == 'R' && buffer[1] == 'S') {
        lastCommand.type = CMD_RESET;
        return;
    }

    // Commands with values (VN, VT, PT, VS, VA, SO)
    if (buffer[0] == 'V' && buffer[1] == 'N') {
        lastCommand.type = CMD_SET_NOMINAL;
        lastCommand.value = parseValue(2);
        lastCommand.has_value = true;
        return;
    }
    if (buffer[0] == 'V' && buffer[1] == 'T') {
        lastCommand.type = CMD_SET_TOLERANCE;
        lastCommand.value = parseValue(2);
        lastCommand.has_value = true;
        return;
    }
    if (buffer[0] == 'P' && buffer[1] == 'T') {
        lastCommand.type = CMD_SET_CUTOFF;
        lastCommand.value = parseValue(2);
        lastCommand.has_value = true;
        return;
    }
    if (buffer[0] == 'V' && buffer[1] == 'S') {
        lastCommand.type = CMD_SET_SAMPLE;
        lastCommand.value = parseValue(2);
        lastCommand.has_value = true;
        return;
    }
    if (buffer[0] == 'V' && buffer[1] == 'A') {
        lastCommand.type = CMD_SET_DELAY;
        lastCommand.value = parseValue(2);
        lastCommand.has_value = true;
        return;
    }
    if (buffer[0] == 'S' && buffer[1] == 'O') {
        lastCommand.type = CMD_SET_OVERRIDE_SPEED;
        lastCommand.value = parseValue(2);
        lastCommand.has_value = true;
        return;
    }

    // Unknown command
    lastCommand.type = CMD_UNKNOWN;
}

float Protocol::parseValue(uint8_t startPos) {
    // Skip any leading whitespace or colon
    while (buffer[startPos] == ' ' || buffer[startPos] == ':') {
        startPos++;
    }
    return atof(&buffer[startPos]);
}

command_t Protocol::getCommand() {
    return lastCommand;
}

void Protocol::sendOk() {
    Serial.println(F("ok"));
}

void Protocol::sendError(const char* msg) {
    Serial.print(F("error: "));
    Serial.println(msg);
}

void Protocol::sendLine(const char* line) {
    Serial.println(line);
}

void Protocol::sendValue(const char* name, float value, int decimals) {
    Serial.print(name);
    Serial.print(F(": "));
    Serial.println(value, decimals);
}

void Protocol::sendValue(const char* name, int value) {
    Serial.print(name);
    Serial.print(F(": "));
    Serial.println(value);
}
