# The Adafruit Feather
Using this board:
https://learn.adafruit.com/adafruit-feather-m0-wifi-atwinc1500/overview

# Getting Started With Feather

1. Plug Feather into Breadboard (not strictly necessary, but is comforting to
  have it be somewhere safe.
1. Plug mini USB into Feather, then the other side into your computer.
1. Download and install the Arduino IDE.  See:
  https://www.arduino.cc/en/Guide/Linux
    * Don't forget to `sudo usermod -a -G dialout $USER`
    * Also, for Ubuntu, needed to fix udev rules:
      https://learn.adafruit.com/adafruit-arduino-ide-setup/linux-setup#udev-rules
1. Follow the adafruit instructions for adding the Feather MO to the Arduino IDE:
  https://learn.adafruit.com/adafruit-feather-m0-basic-proto/setup
1. The blink sketch already exists under ./M0_blink

Note: The CHG LED was blinking.  I thought it might be a defective board but
according to: https://learn.adafruit.com/adafruit-feather-m0-basic-proto/power-management
"This LED might also flicker if the battery is not connected.".

# Test the OLED display

This one:
https://learn.adafruit.com/adafruit-oled-featherwing/pinouts?view=all

Installed libraries at `~/Arduino/libraries`

Also had to:

```
mv Adafruit-GFX-Library Adafruit_GFX
```

Since library folders can't have '-' in them.

Pinout pictures can be found at the link at the top.  Used one strip of 2 for
power and ground, then a strip of 5 for the i2c bus and buttons.  Could have
used one more for the rst, but since it was on breadboard I just used the reset
button on the feather itself.
