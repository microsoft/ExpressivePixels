# Expressive Pixels on Raspberry Pi

## Python support

Running Expressive Pixels with Raspberry Pi allows you to extend the platform using Python. Please follow the steps below for working with Raspberry Pi.

## Hardware
You need the following things for an initial prototype.
* Raspberry Pi (I haven't yet tested Raspberry Pi Zero. But that should work as well)
* A WS2811 or other compatible displays with individually addressable LEDs (e.g. [Sparklet display](https://www.amazon.com/Sparklet-256X-Round-Color-Display/dp/B08FHGGC7N/)
* [TTL Serial Cable](https://www.amazon.com/JBtek-WINDOWS-Supported-Raspberry-Programming/dp/B00QT7LQ88/) (If you find BLE to be buggy and wish to use the serial port)

### Raspberry Pi Setup
* [Install the Raspberry Pi OS](https://www.raspberrypi.org/software/)
* You can connect to your Pi using USB or over the network. Initially USB is convenient, but it is a good idea to make your Raspberry Pi accessible over the network for later use. 
    * [Steps for USB](https://howchoo.com/pi/raspberry-pi-gadget-mode)
    * The main thing here is to make sure that the Pi is connected to your WiFi network and its name can be resolved over the network. Follow these steps for [name resolution from Windows](https://serverfault.com/questions/145994/resolve-linux-hostname-in-windows).
* Setup password less login(http://rebol.com/docs/ssh-auto-login.html)

At this point you should be able to connect to your Raspberri Pi and login. Make sure you have common tools installed like git and Python 3.x.


### Wiring
*NOTE* The Python firmware supports connecting from the Expressive Pixels app using both BLE and Serial ports. The steps below indicate

Please refer to the [GPIO pin layout](https://www.raspberrypi.org/documentation/usage/gpio/) of Raspberry Pi and make the following connections:

-------
|Raspberry Pi Pin       | Connected to   |
------------------------|----------------|
| Pin 2 (5V Power)      |Sparklet Pin 5V |
| Pin 4 (5V Power)      |Sparklet Pin EN |
| Pin 6 (GND)           |Sparklet Pin GND|
| Pin 8 (GPIO 14-TXD)   |TTL cable WHITE |
| Pin 10 (GPIO 15-RXD)  |TTL cable GREEN |
| Pin 12 (GPIO 18-PCM_CLK)|Sparklet Pin DATA(D) |
| Pin 14 (GND)          |TTL cable BLACK (GND)|
------


NOTE: The TTL cable connections are needed only if you are using the TTL cable. As noted above, BLE is supported, but the serial cable can be helpful in isolating connection issues.

## Software
* On the Raspberry Pi
  * `git clone https://github.com/microsoft/ExpressivePixels/`
  * `cd ExpressivePixels/Firmware/RaspberryPi`
  * For BLE connections:
    * `sudo python3 sparklet.py ble`
  * For serial connections:
    * `sudo python3 sparklet.py serial`  

* Install the [Expressive Pixels](https://www.microsoft.com/en-us/p/expressive-pixels/9mtc56w1rxqh?activetab=pivot:overviewtab) app from the Windows Store
* Run and use the Expressive Pixels app

