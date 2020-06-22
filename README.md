# ESP8266 Deauther 3.0

![Build V3](https://github.com/SpacehuhnTech/nightly-deauther/workflows/Build%20V3/badge.svg)

**This version is still in development!**  

[Link to Version 2](https://github.com/SpacehuhnTech/esp8266_deauther/tree/v2/esp8266_deauther)  

Currently implemented:  
- [x] Serial Command Line Interface
- [ ] Web Interface
- [ ] Display (OLED) Interface
 
## Install using .bin file

1. Download latest compiled .bin file from [nightly-deauther/releases](https://github.com/SpacehuhnTech/nightly-deauther/releases)
2. Install [esptool](https://github.com/espressif/esptool/)
3. Connect your ESP8266
4. Flash it by running `esptool.py -p <PORT> -b 115200 write_flash 0 <BIN_FILE>`.  
   Be sure to replace `<PORT>` with the serial port  
   and `<BIN_FILE>` with the path of the previously download .bin file.

## Install using Arduino IDE

1. Install Arduino IDE
2. Install the ESp8266 and Deauther boards as decribed [here](https://github.com/SpacehuhnTech/esp8266_deauther/wiki/Installation#compiling-using-arduino-ide)
3. Install [SimpleCLI Arduino library](https://github.com/spacehuhn/SimpleCLI#installation) (**also available in the Arduino library manager**)
4. Download [V3 source code](https://github.com/SpacehuhnTech/esp8266_deauther/archive/v3.zip) and unzip it
5. Open `esp8266_deauther/esp8266_deauther.ino` with Arduino
6. Select an `ESP8266 Deauther` board in Arduino under `tools` -> `board`
7. Connect your device and select the serial port in Arduino under `tools` -> `port`
8. Click Upload button

## Usage

We recommend using the [Huhnitor](https://github.com/spacehuhntech/huhnitor) for an easy way of interacting with the Deauther serial interface.  
But you can of course use any other serial terminal with 115200baud too.  

## License

This software is licensed under the MIT License. See the [license file](LICENSE) for details.  
