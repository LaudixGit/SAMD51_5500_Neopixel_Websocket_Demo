# SAMD51_5500_Neopixel_Websocket_Demo
Here is a moderately-complex solution controlling color patterns on a neopixel.
This may work on other hardware, but is specifically designed for the [Adafruit SAMD51 M4 Feather](https://www.adafruit.com/product/3857), with a [PoE 5500 network sheild](https://www.amazon.com/gp/product/B08KTVD7BR).
No other hardware is needed.

A basic wepage is provided to select from a few display options:
* OFF - set solor to black and disable the LEDs
* ON - display a constant, specific color
* RAINBOW - display colors across the colorwheel
* FADE - transition between two colors

There are also options when the pattern completes:
* STOP - go to OFF (above)
* TOGGLE - invert the pattern (eg: color1 becomes color2, then color2 returns to color1, repeat)
            (when combined with FADE, results in a pulsing pattern)
* REVERSE - pattern runs backward (eg: color2 becomes color1)
* FORWARD - pattern runs forward (eg: color1 becomes color2)

JSCOLOR library is used when selecting colors.
To send the webpage, there is a workaround used to overcome the 2k-buffer limitation of the Ardunio Ethernet library.
The code is split into multiple files for readability.
The neopixel code is implemented as a class for potential reuse in other sketches.

Up to 3 simultaneous clients can connect, and when any client modifies the settings, all clients receive the update and remain in sync.
(the 5500 can support 8 sockets, so it may be possible 4 clients can simultaneously connect; each requires 2 sockets for HTTP & websocket)
The settings are transferred in JSON format so it is easy to troubleshoot in clear text (Serial.print and console.log)
For convenience, the page header reflects connection status (red = disconnected).


There is no security, encryption, nor authentication.

How to use:
* clone this locally
* open in Ardunio IDE
* install the libraries listed at the top of the INO page
* compile and upload to the SAMD51

This sketch uses the built-in neopixel of the M4 Feather, but by connecting a neopixel strip, and instantiating with more than 1 pixel this code _should_ work with many pixels (up to 2048 with remaining unused memory)

    neopixelLED( <quantity of pixels>,  <pixel-strip control pin> , NEO_GRB + NEO_KHZ800, &neopixelComplete);
