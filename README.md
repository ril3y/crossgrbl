# CrossGRBL

Drop-in replacement controller for the **Langmuir Systems CrossFire** CNC plasma table. Runs on an Arduino Uno with an Electronics-Salon screw terminal shield, fully compatible with Langmuir's **FireControl** sender software.

This project replaces the stock CrossFire controller board with commodity hardware while keeping the full FireControl experience — jogging, G-code streaming, torch control, pierce delay overrides, and THC support.

## What You Need

| Component | Purpose | Approx Cost |
|-----------|---------|-------------|
| Arduino Uno R3 | Motion controller (ATmega328P @ 16MHz) | ~$15 |
| Electronics-Salon Screw Terminal Shield | Breakout for wiring stepper drivers | ~$12 |
| 2x Stepper drivers (DM542, TB6600, etc.) | X and Y axis motor drivers | ~$30 |
| Arduino Nano clone (CH340) | THC controller (optional) | ~$5 |
| Relay module or MOSFET | Torch fire signal | ~$5 |

**Total: ~$65** (vs $200+ for a stock CrossFire replacement board)

## Screw Terminal Shield Wiring

This project uses the **Electronics-Salon D-1229** screw terminal breakout shield, which gives you easy screw-terminal access to every Arduino Uno pin without soldering. ([Amazon link](https://www.amazon.com/dp/B07HF2DD7T))

![Electronics-Salon Screw Terminal Shield](docs/images/screw-terminal-shield.jpg)

The shield plugs directly onto the Arduino Uno. All connections below refer to the **screw terminal numbers** printed on the shield.

### Motor Connections

The CrossFire is a 2-axis (X/Y) plasma table. Each axis needs three wires to its stepper driver: **STEP**, **DIR**, and **ENABLE**.

```
                    STEPPER DRIVER (X)              STEPPER DRIVER (Y)
                   ┌──────────────┐                ┌──────────────┐
Terminal 2  ──────►│ STEP         │  Terminal 3 ──►│ STEP         │
Terminal 5  ──────►│ DIR          │  Terminal 6 ──►│ DIR          │
Terminal 8  ──────►│ ENABLE       │  Terminal 8 ──►│ ENABLE       │
GND         ──────►│ GND          │  GND        ──►│ GND          │
                   └──────────────┘                └──────────────┘
```

| Screw Terminal | Arduino Pin | GRBL Function | Wire To |
|:--------------:|:-----------:|---------------|---------|
| **2** | D2 | X Step | X stepper driver STEP input |
| **3** | D3 | Y Step | Y stepper driver STEP input |
| **5** | D5 | X Direction | X stepper driver DIR input |
| **6** | D6 | Y Direction | Y stepper driver DIR input |
| **8** | D8 | Stepper Enable | Both drivers ENABLE (active low) |
| **GND** | GND | Ground | Both drivers GND (signal ground) |

> **Note:** Terminal 8 (Enable) is shared — run a wire from terminal 8 to both the X and Y driver enable inputs. The enable signal is active-low: LOW = motors engaged, HIGH = motors released.

### Torch Fire (Plasma Trigger)

The torch fire signal uses the **spindle enable** pin. When FireControl sends M3 (spindle on), this pin goes HIGH to trigger your plasma cutter.

| Screw Terminal | Arduino Pin | Function | Wire To |
|:--------------:|:-----------:|----------|---------|
| **12** | D12 | Torch Fire | Relay module IN (or MOSFET gate) |
| **5V** | 5V | Relay power | Relay module VCC |
| **GND** | GND | Ground | Relay module GND |

Connect the relay's normally-open (NO) contacts in parallel with your plasma cutter's manual trigger switch. When FireControl fires the torch (M3), pin 12 goes HIGH, the relay closes, and the plasma fires.

> **Safety:** Always use a relay or optocoupled MOSFET to isolate the Arduino from the plasma cutter's trigger circuit. Never connect the Arduino directly to the plasma trigger.

### Limit Switches (Optional)

| Screw Terminal | Arduino Pin | Function |
|:--------------:|:-----------:|----------|
| **9** | D9 | X Limit |
| **10** | D10 | Y Limit |
| **11** | D11 | Z Limit |

### Control Pins (Optional)

| Screw Terminal | Arduino Pin | Function |
|:--------------:|:-----------:|----------|
| **A0** | A0 | Reset |
| **A1** | A1 | Feed Hold |
| **A2** | A2 | Probe |
| **A5** | A5 | THC Enable Signal |

### Complete Wiring Summary

```
ELECTRONICS-SALON SCREW TERMINAL SHIELD
┌─────────────────────────────────────────────────────┐
│                                                     │
│  Terminal 2  ── X STEP ──────► X Driver STEP        │
│  Terminal 3  ── Y STEP ──────► Y Driver STEP        │
│  Terminal 5  ── X DIR  ──────► X Driver DIR         │
│  Terminal 6  ── Y DIR  ──────► Y Driver DIR         │
│  Terminal 8  ── ENABLE ──┬───► X Driver EN          │
│                          └───► Y Driver EN          │
│  Terminal 12 ── TORCH  ──────► Relay IN             │
│  Terminal A5 ── THC    ──────► THC Module (optional) │
│  GND         ────────────┬───► X Driver GND         │
│                          ├───► Y Driver GND         │
│                          └───► Relay GND            │
│  5V          ────────────────► Relay VCC             │
│                                                     │
└─────────────────────────────────────────────────────┘
```

## Flashing Guide

There are two things to flash:
1. **USB firmware** on the Uno's ATmega16U2 chip (one-time, makes FireControl recognize the device)
2. **GRBL firmware** on the Uno's main ATmega328P chip (the actual motion controller)

### Step 1: Flash USB VID/PID (One-Time)

FireControl checks the USB Vendor ID and Product ID before it will connect. A stock Arduino Uno identifies as VID `2341` PID `0043` — FireControl ignores this. You need to reprogram the Uno's **ATmega16U2 USB interface chip** to report as a Langmuir CrossFire device:

- **Target VID:** `16D0`
- **Target PID:** `0EFB`
- **Device Name:** "Langmuir Systems CrossFire"

#### Entering DFU Mode

The ATmega16U2 is the small chip near the USB port on the Arduino Uno. To reprogram it, you need to put it into DFU (Device Firmware Update) mode:

1. **Locate the ICSP2 header** — This is the 6-pin header closest to the USB port (not the one near the power jack). On most Uno R3 boards it's labeled "ICSP" or "ICSP2".

2. **Briefly short RESET to GND** — Using a jumper wire or tweezers, momentarily connect the RESET pin to the GND pin on the ICSP2 header. The pinout is:
   ```
   ICSP2 Header (near USB port):
   ┌───────────┐
   │ MISO  VCC │
   │ SCK  MOSI │
   │ RST   GND │  ◄── Short these two briefly
   └───────────┘
   ```

3. **Verify DFU mode** — In Device Manager (Windows), you should now see "ATmega16U2 DFU" instead of a COM port. On Linux, `lsusb` will show Atmel DFU device.

#### Flashing with Atmel FLIP (Windows)

1. Download and install [Atmel FLIP](https://www.microchip.com/en-us/development-tool/flip) from Microchip
2. Open FLIP, select device **ATmega16U2**
3. Connect via USB (the DFU device)
4. Load the hex file: `firmware/CrossFire-spoof-16u2.hex`
5. Click **Run** to program
6. Unplug and replug the Arduino

#### Flashing with dfu-programmer (Linux/Mac)

```bash
# Install dfu-programmer
sudo apt install dfu-programmer    # Debian/Ubuntu
brew install dfu-programmer         # macOS

# Put Arduino in DFU mode (short RESET-GND on ICSP2), then:
sudo dfu-programmer atmega16u2 erase
sudo dfu-programmer atmega16u2 flash firmware/CrossFire-spoof-16u2.hex
sudo dfu-programmer atmega16u2 reset
```

#### Verify

After replugging, the Arduino should now appear as:
- **Windows Device Manager:** "Langmuir Systems CrossFire" on a new COM port
- **Linux:** `lsusb` shows `16d0:0efb`
- **Arduino CLI:** `arduino-cli board list` shows the new VID/PID

> **Note:** For Arduino Uno R2 (older, with AT90USB82 instead of ATmega16U2), use `firmware/CrossFire-spoof-8u2.hex` instead.

> **Reverting:** If you need to restore the original Arduino USB firmware, flash the original Arduino Uno USB firmware from the Arduino IDE install directory (`hardware/arduino/avr/firmwares/atmegaxxu2/`).

### Step 2: Flash GRBL Firmware

#### Option A: Using Pre-Built Hex File (Easiest)

```bash
# Using arduino-cli (download from https://arduino.github.io/arduino-cli/)
arduino-cli upload -p COMxx --fqbn arduino:avr:uno --input-file firmware/grbl-ls-1.3ls.hex
```

Replace `COMxx` with your actual COM port (check Device Manager).

#### Option B: Using avrdude Directly

```bash
avrdude -c arduino -p atmega328p -P COMxx -b 115200 -U flash:w:firmware/grbl-ls-1.3ls.hex:i
```

#### Option C: Build From Source

Requires `avr-gcc` toolchain installed.

```bash
# Build
make

# Flash (uses avrisp2 programmer by default)
make flash

# Or specify Arduino bootloader programmer:
make flash PROGRAMMER="-c arduino -P COMxx -b 115200"
```

### Step 3: Verify in FireControl

1. Open FireControl
2. It should detect **"CrossFire Gen 2"** on the COM port
3. Status bar shows connection and version **1.3ls**
4. Try jogging — if the motors move in the correct direction, you're done
5. If an axis moves backwards, flip the DIR wires on that stepper driver (or change `$3` direction invert mask)

## GRBL Settings (CrossFire Defaults)

These are baked into the firmware but can be changed at runtime via serial commands (`$x=value`):

| Setting | Value | Description |
|---------|-------|-------------|
| `$0` | 10 | Step pulse time (microseconds) |
| `$1` | 255 | Step idle delay (255 = always on) |
| `$100` | 125.984 | X steps/mm |
| `$101` | 125.984 | Y steps/mm |
| `$102` | 266.666 | Z steps/mm |
| `$110` | 7620 | X max rate (mm/min) |
| `$111` | 7620 | Y max rate (mm/min) |
| `$112` | 3810 | Z max rate (mm/min) |
| `$120` | 980 | X acceleration (mm/sec^2) |
| `$121` | 980 | Y acceleration (mm/sec^2) |
| `$122` | 980 | Z acceleration (mm/sec^2) |

## THC (Torch Height Controller) — Optional

The `thc-firmware/` directory contains firmware for a separate Arduino Nano that acts as a Torch Height Controller, replacing the $750 Langmuir LS-THC module with a ~$15 DIY solution.

Arduino Nano clones with CH340 USB chips have VID `1A86` PID `7523`, which matches the Langmuir LS-THC — **no USB spoofing needed**.

See [thc-firmware/README.md](thc-firmware/README.md) for full wiring and setup instructions.

## FireControl Protocol Reference

### Pierce Delay Override Commands

| Byte | Command | Description |
|------|---------|-------------|
| `0xA2` | DWELL_50 | Pierce delay 50% |
| `0xA3` | DWELL_100 | Pierce delay 100% |
| `0xA4` | DWELL_150 | Pierce delay 150% |
| `0xA5` | DWELL_200 | Pierce delay 200% |
| `0xA6` | DWELL_0 | Pierce delay 0% (skip) |
| `0xA9` | CANCEL_WAIT | Cancel current dwell immediately |

### Key Integration Points

- **Startup banner:** `Grbl 1.3ls` (FireControl checks this)
- **Baud rate:** 115200
- **Status reports:** `<Idle|MPos:0.000,0.000,0.000|...|Ov:100,100,100,100>` (4 override values — the 4th is the dwell override)
- **Torch control:** M3 = torch on, M5 = torch off (mapped to spindle enable)
- **Jogging:** `$J=G91 X10 F1000` style commands

## Directory Structure

```
crossgrbl/
├── grbl/                  # GRBL source code (ATmega328P)
│   ├── config.h           # Compile-time config (CrossFire defaults)
│   ├── cpu_map.h          # Pin assignments
│   ├── defaults.h         # Machine defaults (steps/mm, speeds, etc.)
│   └── ...                # Core GRBL source files
├── thc-firmware/          # THC controller firmware (Arduino Nano)
├── firmware/              # Pre-built hex files
│   ├── grbl-ls-1.3ls.hex              # Ready-to-flash GRBL firmware
│   ├── CrossFire-spoof-16u2.hex       # USB spoof for Uno R3 (ATmega16U2)
│   ├── CrossFire-spoof-8u2.hex        # USB spoof for Uno R2 (AT90USB82)
│   ├── THC-spoof-8u2.hex             # USB spoof for THC (if needed)
│   └── CrossFire-Gen2-v1.3ls-official.hex  # Stock Langmuir firmware (reference)
├── docs/images/           # Documentation images
├── Makefile               # Build/flash GRBL from source
├── COPYING                # GPLv3 License
└── README.md
```

## Troubleshooting

### FireControl won't detect the board
- Verify USB VID/PID was flashed correctly: check Device Manager for "Langmuir Systems CrossFire"
- If it still shows as "Arduino Uno", the USB firmware flash didn't take — retry DFU mode
- Make sure you're using the correct hex file (16u2 vs 8u2)

### Motors don't move
- Check terminal 8 (Enable) is wired to both drivers — it must be LOW to enable
- Verify STEP and DIR wires are on the correct terminals (2/5 for X, 3/6 for Y)
- Check your stepper driver power supply is on

### Axis moves in wrong direction
- Swap the DIR wire polarity on the stepper driver, OR
- Change the direction invert mask: `$3=1` (invert X), `$3=2` (invert Y), `$3=3` (invert both)

### Torch won't fire
- Check terminal 12 is wired to relay input
- Verify relay VCC is connected to 5V and GND to GND
- Test manually: send `M3` via serial terminal, pin 12 should go HIGH
- Check relay NO contacts are wired across plasma trigger switch

### Version mismatch warning in FireControl
- Ensure GRBL reports `1.3ls` — connect via serial at 115200 baud and check startup banner
- If you built from source, verify `GRBL_VERSION` in `grbl/grbl.h` is `"1.3ls"`

## License

GRBL firmware is licensed under the **GNU General Public License v3.0** — see [COPYING](COPYING).

THC firmware is licensed under the **MIT License**.

## Credits

- [GRBL](https://github.com/gnea/grbl) by Sungeun K. Jeon / Gnea Research LLC
- [grbl-ls](https://github.com/langmuirsystems/grbl-ls) by Langmuir Systems
- [Electronics-Salon](https://www.electronics-salon.com/) for the screw terminal shield
