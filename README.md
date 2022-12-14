# pico-satellite

pico-satellite is an arduino script for the Raspberry Pi Pico W and the Pimoroni Pico Unicorn 16x7 display that can retrieve satellite position data and displays it on a world map. 

![Working project](/assets/main.jpg)

It was inspired by [John Chinner's project on Twitter](https://twitter.com/JohnChinner/status/1554912401253883904). 
The obvious difference between the projects is that John's is written in MicroPython and uses the ISS-info library. 
I don't know MicroPython, nor do I much care for the idea of porting the library to C++ and/or learning the orbital mechanics required to develop a tracking library for any satellite given the TLE. 
Consequntly, this project just uses a simple API.

With the v1.1.0 release, 4 different satellites are abled to be tracked with the buttons controlling which satellite is currently in view.

With the v1.2.0 release, the display can create a trail for the satellite, showing the orbital pattern appearing in real time. To engage the hold view mode:
* Press and hold one of the satellite buttons for more than 2 seconds, and release.
* The current satellite location will blink twice in success
* The display will now create a trail of red at each location of the satellite, showing the orbital pattern
* To cancel, just refresh the display by clicking one of the buttons.


![Tracking the ISS](/assets/iss_track.jpg)

## Software

Satellite data is retrived from the [n2yo API](https://www.n2yo.com/api/). It is capable of tracking any satellite with the obvious restriction that it must be on ny2o.

The API request code is adapted from [Brian Loughs sample API request](https://github.com/witnessmenow/arduino-sample-api-request) code with some very minor tweaks.

The pico W is written to with the Arduino IDE made possible by using Earle F. Philhower III's great [arduino-pico](https://github.com/earlephilhower/arduino-pico). 
This includes its own WiFi library that is similar to the ESP8266 WiFi library used in Brian's sample code. For more information, [see here](https://arduino-pico.readthedocs.io/en/latest/wifi.html).

As with Brian's code, [ArduinoJson](https://arduinojson.org/) is used to parse the JSON data into usable information.

[Bodmer's Unicorn GFX library](https://github.com/Bodmer/Pico_Unicorn_GFX) is key in displaying to the Unicorn display.

## Hardware
- [Raspberry Pi Pico W](https://shop.pimoroni.com/products/raspberry-pi-pico-w)
- [Pimoroni Pico Unicorn](https://shop.pimoroni.com/products/pico-unicorn-pack)

## Instructions for use

1. Download the files however you like
2. Open the pico-satellite.ino file with the Arduino IDE
3. Edit the ssid and password variables and replace `YOUR_NETWORK_SSID_HERE` and `YOUR_NETWORK_KEY_HERE` with your own respective information
4. Go to [www.n2yo.com/api/](www.n2yo.com/api/) and register for an account. 
5. Log in, and get your API key
6. Replace `YOUR_API_KEY_HERE` with your own API key from n2yo
7. Replace `satID0`, `satID1`, `satID2`, and `satID3` with the satellite NORAD ID's from n2yo that you want to track e.g. 25544 for the ISS. There are default set values already.
8. Upload to the pico and enjoy!
