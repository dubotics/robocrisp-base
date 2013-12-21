# Basic controls test

This directory contains client/server/slave (base station/Raspberry Pi/Arduino) code for testing
basic communication and control functionality.

  * `rgb-i2c` contains the Arduino sketch.  This sketch allows control via I²C of each channel
    (color) of an RGB LED attached to an Arduino on PWM pins 11 (red), 10 (green), and 9 (blue).
    If the sketch doesn't receive any commands for a set amount of time, it switch to an "idle
    mode" in which it cycles the LED's color.

  * `node.cc` implements the client and server functionalities; it was copied from RoboCRISP's
    `tests/live-test.cc` and modified to provide the appropriate interfaces and behaviour for
    our purposes here.
