# right_bbtrackpad

Right-side BlackBerry-style A320 pointing module for the ZitaoTech Sofle.

This shield intentionally reuses the existing `trackpoint_split` channel on the
right half. The left central already has the corresponding listener and this
keeps the left local `left_bbtrackpad` listener independent when both halves use
A320 modules.

Initial hardware assumptions are inherited from the known-working
`right_trackpoint` module connector:

- SDA: P0.07
- SCL: P1.08
- MOTION_N: P0.14
- LED PWM: P0.04
- A320 I2C address: 0x3B

If pointer direction is reversed after real-hardware testing, adjust X/Y signs
in `custom_driver_right/a320.c`; do not rewire the module first.
