# pico-satellite

pico-satellite is an arduino script for the Raspberry Pi Pico W and the Pimoroni Pico Unicorn 16x7 display that can retrieve satellite position data and displays it on a world map. 

It was inspired by [John Chinner's project on Twitter](twitter.com/JohnChinner/status/1554912401253883904). The obvious difference between the projects is such: John's is written in MicroPython and uses the ISS-info library. I don't know MicroPython, nor do I much care for the idea of porting the library to C++ and/or learning the orbital mechanics required to develop a tracking library for any satellite given the TLE. As such, a simple API will suffice.

Satellite data is retrived from the [n2yo API](www.n2yo.com/api/). It is capable of tracking any satellite with the obvious restriction that it must be on ny2o.

The API request code is adapted from [Brian Loughs sample API request](https://github.com/witnessmenow/arduino-sample-api-request) code with some very minor tweaks.

The pico W is written to with the Arduino IDE made possible by using Earle F. Philhower III's great [arduino-pico](github.com/earlephilhower/arduino-pico). This includes its own WiFi library that is similar to the ESP8266 WiFi library used in Brian's sample code. For more information, [see here](arduino-pico.readthedocs.io/en/latest/wifi.html).

As with Brian's code, [ArduinoJson](arduinojson.org/) is used to parse the JSON data into usable information.

## Hardware
- [Raspberry Pi Pico W](shop.pimoroni.com/products/raspberry-pi-pico-w)
- [Pimoroni Pico Unicorn](shop.pimoroni.com/products/pico-unicorn-pack)

## Instructions for use

1. Download the files however you like
2. Open the pico-satellite.ino file with the Arduino IDE
3. Edit the ssid and password variables and replace `YOUR_NETWORK_SSID_HERE` and `YOUR_NETWORK_KEY_HERE` with your own respective information
4. Go to [www.n2yo.com/api/](www.n2yo.com/api/) and register for an account. 
5. Log in, and get your API key
6. Scroll down the code file until you reach the line `client.print("/rest/v1/satellite/positions/SAT_ID/50/-1/0/1/&apiKey=YOUR_API_KEY_HERE");`
7. Replace `YOUR_API_KEY_HERE` with your own API key from n2yo
8. Replace `SAT_ID` with the satellite NORAD ID from n2yo that you want to track e.g. 25544 for the ISS
9. Upload to the pico and enjoy!
