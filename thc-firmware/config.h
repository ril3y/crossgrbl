/*
 * config.h - THC Firmware Configuration
 *
 * Pin definitions, constants, and compile-time settings for
 * FireControl-compatible Torch Height Controller.
 */

#ifndef CONFIG_H
#define CONFIG_H

// =============================================================================
// VERSION
// =============================================================================

#define THC_VERSION "LS-THC v1.0"

// =============================================================================
// PIN DEFINITIONS
// =============================================================================

// Analog inputs
#define PIN_ARC_VOLTAGE     A0    // Arc voltage input (via 200:1 divider)

// Digital inputs (for speed detection)
#define PIN_X_STEP          2     // X axis step pulse (INT0)
#define PIN_Y_STEP          3     // Y axis step pulse (INT1)

// Digital outputs (Z axis control)
#define PIN_Z_STEP          4     // Z stepper step output
#define PIN_Z_DIR           5     // Z stepper direction output
#define PIN_Z_ENABLE        6     // Z stepper enable output (active low)

// Status LED
#define PIN_STATUS_LED      13    // Built-in LED

// =============================================================================
// SERIAL CONFIGURATION
// =============================================================================

#define SERIAL_BAUD         115200
#define SERIAL_TIMEOUT_MS   100
#define CMD_BUFFER_SIZE     32

// =============================================================================
// VOLTAGE SENSING
// =============================================================================

// ADC configuration
#define ADC_REFERENCE       5.0   // Arduino reference voltage
#define ADC_RESOLUTION      1024  // 10-bit ADC
#define VOLTAGE_DIVIDER     200.0 // Divider ratio (200:1)

// Filtering
#define VOLTAGE_SAMPLES     16    // Rolling average sample count
#define VOLTAGE_READ_INTERVAL_US 1000  // Read ADC every 1ms

// =============================================================================
// Z AXIS MOTION
// =============================================================================

// Step timing (adjust for your stepper driver)
#define STEP_PULSE_US       5     // Minimum step pulse width
#define STEP_INTERVAL_US    500   // Minimum time between steps (max speed)

// Direction constants
#define Z_DIR_UP            HIGH  // Direction pin state for UP
#define Z_DIR_DOWN          LOW   // Direction pin state for DOWN

// Enable logic (most drivers are active-low)
#define Z_ENABLED           LOW
#define Z_DISABLED          HIGH

// =============================================================================
// THC CONTROL PARAMETERS (defaults, stored in EEPROM)
// =============================================================================

// Voltage settings
#define DEFAULT_NOMINAL_VOLTAGE   120.0  // Target arc voltage (V)
#define DEFAULT_VOLTAGE_TOLERANCE 2.0    // Deadband tolerance (V)

// Speed cutoff
#define DEFAULT_SPEED_CUTOFF      50     // % of programmed speed to freeze THC

// Smart voltage sampling
#define DEFAULT_SAMPLE_SIZE       10     // Smart voltage sample size
#define DEFAULT_SAMPLE_DELAY      100    // Sample delay (ms after arc start)

// Override speed
#define DEFAULT_OVERRIDE_SPEED    10     // Override mode speed (steps/sec)

// =============================================================================
// ANTI-DIVE SPEED DETECTION
// =============================================================================

#define SPEED_CALC_INTERVAL_MS    50     // Calculate speed every 50ms
#define SPEED_HISTORY_SIZE        4      // Samples for speed averaging

// =============================================================================
// STATE MACHINE
// =============================================================================

// THC States
typedef enum {
    STATE_IDLE = 0,
    STATE_ENABLED,
    STATE_TORCH_ON,
    STATE_ARC_OK,
    STATE_FROZEN,
    STATE_ERROR
} thc_state_t;

// Bitflags for status response
#define FLAG_MOVING_UP      0x01  // Bit 1: Moving up
#define FLAG_AT_TARGET      0x02  // Bit 2: At target (OK)
#define FLAG_MOVING_DOWN    0x04  // Bit 3: Moving down
#define FLAG_ACTIVE         0x08  // Bit 4: THC active
#define FLAG_SMART_MODE     0x10  // Bit 5: Smart voltage mode
#define FLAG_OVERRIDE       0x20  // Bit 6: Override mode
#define FLAG_ARC_OK         0x40  // Bit 7: Arc OK detected

// Error codes
#define ERROR_NONE          0
#define ERROR_LOST_ARC      1
#define ERROR_TIMEOUT       2
#define ERROR_SETTINGS      3

// =============================================================================
// TIMING
// =============================================================================

#define ARC_OK_VOLTAGE      50.0  // Minimum voltage to consider arc established
#define ARC_OK_DELAY_MS     100   // Time after torch on before checking arc
#define LOST_ARC_VOLTAGE    30.0  // Voltage below which arc is considered lost
#define LOST_ARC_DELAY_MS   50    // Time below threshold before error

// =============================================================================
// EEPROM
// =============================================================================

#define EEPROM_MAGIC        0x5448  // Magic bytes "TH" to validate EEPROM
#define EEPROM_VERSION      1     // Settings version for migration

// EEPROM addresses
#define EEPROM_ADDR_MAGIC   0
#define EEPROM_ADDR_DATA    2

// =============================================================================
// DEBUG (uncomment to enable)
// =============================================================================

// #define DEBUG_SERIAL
// #define DEBUG_VOLTAGE
// #define DEBUG_MOTION
// #define DEBUG_STATE

#endif // CONFIG_H
