# THC Firmware - FireControl Compatible Torch Height Controller

Open-source Arduino-based Torch Height Controller firmware compatible with Langmuir FireControl software.

## Overview

This firmware replaces the $750 Langmuir LS-THC module with a ~$15 DIY solution using an Arduino Nano clone. It implements the same serial protocol that FireControl expects, allowing seamless integration.

## Hardware Requirements

| Component | Purpose | Cost |
|-----------|---------|------|
| Arduino Nano clone with CH340 | Main controller | ~$5 |
| Voltage divider circuit | Arc voltage sensing | ~$5 |
| Z stepper driver (optional) | Direct Z control | ~$5-10 |

**Total: ~$15-20**

### Why CH340?

Arduino Nano clones with CH340 USB chips have VID:1A86 PID:7523 which matches the Langmuir LS-THC. FireControl recognizes these devices without any modification.

## Wiring

### Pin Connections

| Arduino Pin | Function | Connection |
|-------------|----------|------------|
| A0 | Arc voltage | Voltage divider output |
| D2 | X step pulse | GRBL X step signal |
| D3 | Y step pulse | GRBL Y step signal |
| D4 | Z step output | Z stepper driver STEP |
| D5 | Z direction | Z stepper driver DIR |
| D6 | Z enable | Z stepper driver EN |
| D13 | Status LED | Built-in LED |

### Voltage Divider Circuit

```
Arc Voltage (50-300V)
         │
        [R1] 1MΩ (or 2x 500kΩ)
         │
         ├──────── To A0
         │
        [R2] 5kΩ
         │
        GND
```

**Ratio:** 200:1 (1MΩ / 5kΩ)
**Example:** 100V arc → 0.5V at Arduino

**Safety:** Use properly rated resistors (1/2W minimum). Consider adding a 5.1V Zener diode to protect the Arduino input.

## Building

### Arduino IDE

1. Open `thc-firmware.ino` in Arduino IDE
2. Select Board: "Arduino Nano"
3. Select Processor: "ATmega328P" (or "Old Bootloader" for clones)
4. Select Port: Your COM port
5. Click Upload

### Arduino CLI

```bash
arduino-cli compile --fqbn arduino:avr:nano thc-firmware
arduino-cli upload -p COM3 --fqbn arduino:avr:nano thc-firmware
```

### PlatformIO

Create `platformio.ini`:
```ini
[env:nano]
platform = atmelavr
board = nanoatmega328
framework = arduino
```

Then: `pio run -t upload`

## Protocol

### Startup Banner
```
LS-THC v1.0
```

### Commands (FireControl → THC)

| Command | Description |
|---------|-------------|
| `?` | Request status |
| `$` | Request settings |
| `ON` | Enable THC |
| `OF` | Disable THC |
| `TO` | Torch on signal |
| `TF` | Torch off signal |
| `VN<v>` | Set nominal voltage |
| `VT<v>` | Set voltage tolerance |
| `PT<v>` | Set speed cutoff % |
| `VS<v>` | Set sample size |
| `VA<v>` | Set sample delay |
| `SO<v>` | Set override speed |
| `ZU` | Override up |
| `ZD` | Override down |
| `ZS` | Override stop |
| `SM` | Smart mode on |
| `SF` | Smart mode off |
| `CE` | Clear error |
| `SV` | Save settings |
| `RS` | Reset to defaults |

### Status Response Format
```
<state>|<liveV>|<nominalV>|<tolerance>|<speed>|<progSpeed>|<cutoff>|<bitflags>|<error>
ok
```

Example:
```
3|125.5|120.0|2.0|95|100|50|66|0
ok
```

### Settings Response Format
```
VS: 10
VT: 2.0
VN: 120.0
PT: 50
VA: 100
SO: 10
SM: 0
ok
```

## State Machine

```
IDLE ──(ON)──► ENABLED ──(TO)──► TORCH_ON ──(arc_ok)──► ARC_OK
  ▲                                                        │
  │                                                        ▼
  └────────────────(OF/error)──────────────────────── FROZEN
```

### States

| State | Code | Description |
|-------|------|-------------|
| IDLE | 0 | THC disabled |
| ENABLED | 1 | THC enabled, waiting for torch |
| TORCH_ON | 2 | Torch firing, waiting for arc |
| ARC_OK | 3 | Arc established, THC active |
| FROZEN | 4 | Speed below cutoff, THC paused |
| ERROR | 9 | Error condition |

### Bitflags

| Bit | Value | Meaning |
|-----|-------|---------|
| 1 | 0x01 | Moving up |
| 2 | 0x02 | At target |
| 3 | 0x04 | Moving down |
| 4 | 0x08 | THC active |
| 5 | 0x10 | Smart mode |
| 6 | 0x20 | Override mode |
| 7 | 0x40 | Arc OK |

## Configuration

Edit `config.h` to customize:

- Pin assignments
- Voltage divider ratio
- ADC filtering parameters
- Default settings
- Timing constants

## Testing

### Bench Test with Potentiometer

1. Connect a 10kΩ potentiometer to A0 (wiper), 5V, and GND
2. Upload firmware
3. Open serial monitor at 115200 baud
4. Send `?` to see voltage readings
5. Turn pot to simulate arc voltage changes
6. Send `ON`, `TO` to enable THC
7. Observe state transitions

### Serial Test Commands

```
?       # Check status
$       # Check settings
ON      # Enable THC
VN120   # Set nominal to 120V
VT2.5   # Set tolerance to 2.5V
SV      # Save settings
?       # Verify changes
```

## Troubleshooting

### FireControl doesn't detect THC

- Verify CH340 drivers are installed
- Check Device Manager for VID:1A86 PID:7523
- Ensure correct COM port selected in FireControl

### No voltage reading

- Check voltage divider wiring
- Verify A0 connection
- Test with known voltage source

### Z motor doesn't move

- Check step/dir/enable wiring
- Verify stepper driver power
- Check enable polarity in config.h

### THC dives at corners

- Increase PT (speed cutoff) value
- Verify X/Y step pulse connections
- Check interrupt pins (D2, D3)

## License

MIT License - See LICENSE file

## Credits

Reverse-engineered from Langmuir FireControl/LS-THC protocol analysis.
