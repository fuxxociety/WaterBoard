# waterboard-old
Arduino-based RO/DI water generator

This revision only consists of:
Float Switches (3)
2-Relay board (1)
12V solenoids (2)

Each solenoid MUST have a flyback diode installed. Otherwise, the High-High sensor is tripped when the relays are powered off.

Only two relays are mandatory for the RO/DI unit operation. The other relays in the code are not currently attached.
