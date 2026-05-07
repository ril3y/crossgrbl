; CrossFire Axis Test - Square Pattern
; Tests X and Y axis direction without torch firing
; Machine should move in a 100mm square starting from home

G21         ; Set units to millimeters
G90         ; Absolute positioning
G94         ; Feed rate in units/min

G0 X0 Y0    ; Move to origin
G4 P1       ; Pause 1 second

; Draw 100mm square (no torch fire)
G1 X100 Y0 F1000    ; Move RIGHT 100mm (+X direction)
G4 P0.5             ; Pause 0.5 sec

G1 X100 Y100 F1000  ; Move AWAY/BACK 100mm (+Y direction)
G4 P0.5             ; Pause 0.5 sec

G1 X0 Y100 F1000    ; Move LEFT 100mm (-X direction)
G4 P0.5             ; Pause 0.5 sec

G1 X0 Y0 F1000      ; Move TOWARD/FRONT 100mm (-Y direction)
G4 P0.5             ; Pause 0.5 sec

; Return to origin
G0 X0 Y0

M2          ; Program end
