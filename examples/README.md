# Example G-code Files

Test files for verifying your CrossFire setup with CrossGRBL firmware.

## Test Files

### test-square.nc
**Purpose:** Axis calibration and direction test
**Description:** 4" square pattern using G1 moves at 120 IPM (no torch firing)
**Use case:**
- Verify X/Y axis directions are correct
- Test movement calibration (should measure exactly 4" x 4")
- Confirm max speed settings are working

**Expected behavior:**
- Starts at origin (0,0)
- Moves RIGHT 4" (+X)
- Moves AWAY 4" (+Y)
- Moves LEFT 4" (-X)
- Moves TOWARD 4" (-Y)
- Returns to origin

### test-2inch-circle.nc
**Purpose:** Actual cutting test with torch firing
**Description:** 2" diameter circle cut from Fusion 360
**Use case:**
- First real cut test
- Verify torch on/off (M3/M5) commands work
- Test diagonal/circular motion
- Verify post-flow air operation

**Settings:**
- Generated with Fusion 360 CrossFire post processor
- Includes proper IHS/THC settings (set to No if not installed)
- Cutting speed: 1000 mm/min (~40 IPM)

## How to Use

1. **Start with test-square.nc** (no torch fire)
   - Load in FireControl
   - Jog to a safe starting position
   - Run the program
   - Verify the gantry traces a 4" square
   - Measure to confirm calibration

2. **Then try test-2inch-circle.nc** (torch fires)
   - Set up material and work clamp
   - Ensure torch is at proper height
   - Load program in FireControl
   - Run and observe torch firing/cutting
   - Check cut quality and dimensions

## Safety Notes

- Always have proper eye protection when torch fires
- Ensure work clamp is connected
- Have adequate ventilation
- Keep flammable materials away from cutting area
- Test torch firing manually (M3/M5 in console) before running programs
