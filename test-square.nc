; CrossFire Axis Test - 4 inch Square Pattern
; Tests X and Y axis direction without torch firing
; Machine should move in a 4" square starting from home

G20         ; Set units to inches
G90         ; Absolute positioning
G94         ; Feed rate in units/min
F120        ; Set feed rate to 120 inches/min

G0 X0 Y0    ; Move to origin
G4 P1       ; Pause 1 second

; Draw 4" square (no torch fire - using G1 moves only)
G1 X4 Y0    ; Move RIGHT 4 inches (+X direction)
G4 P0.5     ; Pause 0.5 sec

G1 X4 Y4    ; Move AWAY/BACK 4 inches (+Y direction)
G4 P0.5     ; Pause 0.5 sec

G1 X0 Y4    ; Move LEFT 4 inches (-X direction)
G4 P0.5     ; Pause 0.5 sec

G1 X0 Y0    ; Move TOWARD/FRONT 4 inches (-Y direction)
G4 P0.5     ; Pause 0.5 sec

; Return to origin
G0 X0 Y0

M2          ; Program end
