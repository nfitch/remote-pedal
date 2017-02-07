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

Load `oled_test`.  You should see something like this on the OLED:

```
   Hello World!
 <=> <=> <=> <=>
```

# Test the OLED display with a Potentiometer

Tutorial:
https://www.arduino.cc/en/tutorial/potentiometer

* Left pin: Ground
* Middle  : Input Pin (A5 in my case)
* Right   : 3.3v

Wire up the potentiometer to pin 5.  Then load `oled_pot`.

# Test Wifi Module

Tutorial is here:
https://learn.adafruit.com/adafruit-feather-m0-wifi-atwinc1500/using-the-wifi-module

1. Install the wifi101 library
1. Verify the firmware version (see above).  Make sure you've `WiFi.setPins`.
1. Follow the other instructions to scan for networks, etc.
1. In oled_pot_wifi sketch folder create wifi credentials (see below)
1. Load `oled_pot_wifi`, open the console and see that it has connected to WiFi.
1. Ping the ip address of your feather.  It will ack.
1. `curl` the address of your feather and it will respond with the potentiometer
   reading.

```
$ ping 192.168.2.142
PING 192.168.2.142 (192.168.2.142) 56(84) bytes of data.
64 bytes from 192.168.2.142: icmp_seq=1 ttl=64 time=367 ms
64 bytes from 192.168.2.142: icmp_seq=2 ttl=64 time=7.51 ms
64 bytes from 192.168.2.142: icmp_seq=3 ttl=64 time=7.76 ms
$ curl -v http://192.168.2.142
* Rebuilt URL to: http://192.168.2.142/
* Hostname was NOT found in DNS cache
*   Trying 192.168.2.142...
* Connected to 192.168.2.142 (192.168.2.142) port 80 (#0)
> GET / HTTP/1.1
> User-Agent: curl/7.38.0
> Host: 192.168.2.142
> Accept: */*
>
< HTTP/1.1 200 OK
< Content-Type: text/plain
< Connection: close
<
53
* Closing connection 0
```

Creating wifi credentials:
```
$ cp ./wifi_creds.ino.example ./oled_pot_wifi/wifi_creds.ino
$ emacs ./oled_pot_wifi/wifi_creds.ino
# Replace the ssid and pass.  As long as that file is in the same directory
# as oled_pot_wifi.ino, it'll get picked up at compile time.
```

# Pedal Test

Get a TRS Jack.  Wire the sleeve to ground.  Wire the tip to GPIO 11 and the
Ring to GPIO 12.

1. Install Bounce2 Library
    * http://playground.arduino.cc/Code/Bounce
    * https://github.com/thomasfredericks/Bounce2/wiki
1. Open pedal_test/pedal_test.ino
1. Fix the _BV comilation error (see below)
1. Open the console after uploading the sketch.
1. Press the pedal buttons.  See the console output.

## Test Pedal Gotchas

After installing Bounce 2 there were some errors on compile:

```
state = _BV(DEBOUNCED_STATE) | _BV(UNSTABLE_STATE);
```

This gave the answer: https://forum.arduino.cc/index.php?topic=266132.30

```
// Added to ~/Arduino/libraries/Bounce2/Bounce2.cpp
#ifndef _BV
#define _BV(x) (1 << (x))
#endif
```
# Websocket Lib Install/Test

Install the Arduino Websocket Server into your libraries directory:
```
~/Arduino/libraries$ git clone git@github.com:nfitch/Arduino-Websocket.git
```

Then:

1. Open websocket_wifi/websocket_wifi.ino
1. Upload the sketch and open the console
1. See the console and OLED display indicate it is connected to wifi
1. Take note of the IP address of the microcontroller
1. Open http://www.websocket.org/echo.html in a browser
1. Point "Location:" at ```ws://<ip address>```
1. Hit the "Connect" button

See on the Serial Console, once per second:
```
Sent data: hello world!
```

See on the Echo Test webpage, once per second:
```
RECEIVED: hello world!
```

You can also have the Echo Test webpage send a message and see it printed on the
Serial Console.

# Install Remote Pedal

If you've been following along with all the above you should already have the
pedal parts assembled.  At that point you can test the remote pedal by:

1. Open the remote_pedal/remote_pedal.ino
1. Upload the sketch and open the console
1. See the console and OLED display indicate it is connected to wifi
1. Take note of the IP address of the microcontroller
1. Open http://www.websocket.org/echo.html in a browser
1. Point "Location:" at ```ws://<ip address>```
1. Hit the "Connect" button
1. Press the buttons on the pedal

See on the serial console:
```
Handshake Successful
Button pressed: right
Sent data: {"v":0,"b":"right","d":4,"p":14}
Button pressed: middle
Sent data: {"v":0,"b":"middle","d":2,"p":14}
Button pressed: left
Sent data: {"v":0,"b":"left","d":0,"p":15}
```

See on the Echo Test webpage:
```
CONNECTED
RECEIVED: {"v":0,"b":"right","d":4,"p":14}
RECEIVED: {"v":0,"b":"middle","d":2,"p":14}
RECEIVED: {"v":0,"b":"left","d":0,"p":15}
```

Note:
1. ```v```: Version is 0
1. ```b```: Button (right, left, middle)
1. ```d```: Duration the button was held, in seconds
1. ```p```: Potentiometer reading
