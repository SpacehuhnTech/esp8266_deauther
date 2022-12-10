# ESP8266 Deauther 3.0

**This version lacks a Web Interface and support for a Display (OLED)** [Link to Version 2](https://github.com/SpacehuhnTech/esp8266_deauther/tree/v2/esp8266_deauther)  

**For tutorials check out our [Blog](https://blog.spacehuhn.com/series/deautherv3)**

## What's new

👉 You can now use [serial.huhn.me](https://serial.huhn.me), our web based serial terminal, for Deauther V3. 

- CLI
  - Powerful new command line interface
  - Support for the [Huhnitor](https://github.com/spacehuhntech/huhnitor) an easy to use, cross platform serial terminal
  - User friendly `start` command to run complex commands step by step
- Attack
  - More options for specific targeting
  - Advertise wifi networks using beacons to a single or a set of devices
  - Deauth a device on multiple channels
- Scan
  - Discover probe requests from devices nearby, disclosing what networks they have previously connected to
  - See MAC addresses of devices which try to connect to networks advertised by the beacon attack
  - Alias MAC addresses to easily recognize known devices
 
## Install using .bin file

1. Download latest compiled .bin file from [nightly-deauther/releases](https://github.com/SpacehuhnTech/nightly-deauther/releases)
2. Install [esptool](https://github.com/espressif/esptool/)
3. Connect your ESP8266
4. Flash it by running `esptool.py -p <PORT> -b 115200 write_flash 0 <BIN_FILE>`.  
   Be sure to replace `<PORT>` with the serial port  
   and `<BIN_FILE>` with the path of the previously download .bin file.

## Install using Arduino IDE

1. Install Arduino IDE
2. In Arduino go to `File` -> `Preferences` add both URLs to `Additional Boards Manager URLs`
   `https://raw.githubusercontent.com/SpacehuhnTech/arduino/main/package_spacehuhn_index.json`  
3. In Arduino go to `Tools` -> `Board` -> `Boards Manager` search for and install the `deauther` package  
4. Download [V3 source code](https://github.com/SpacehuhnTech/esp8266_deauther/archive/v3.zip) and unzip it  
   or `git clone https://github.com/SpacehuhnTech/esp8266_deauther`, `cd esp8266` and `git checkout v3`
5. Open `esp8266_deauther/esp8266_deauther.ino` with Arduino
6. Select an `ESP8266 Deauther` board in Arduino under `tools` -> `board`
7. Connect your device and select the serial port in Arduino under `tools` -> `port`
8. Click Upload button

## Install using Arduino-CLI

1. [Install Arduino-CLI](https://arduino.github.io/arduino-cli/installation/)  
   `brew install arduino-cli`
2. Download [V3 source code](https://github.com/SpacehuhnTech/esp8266_deauther/archive/v3.zip) and extracting it  
   or `git clone https://github.com/SpacehuhnTech/esp8266_deauther`,  
   `cd esp8266` and  
   `git checkout v3`
3. Update board URLs `arduino-cli core update-index`
4. Install [ESP8266 Deauther Core](https://github.com/tobozo/Arduino/)  
   `arduino-cli core install deauther:esp8266`
5. Compile  
  `cd esp8266_deauther`  
  `arduino-cli compile esp8266_deauther --fqbn deauther:esp8266:d1_mini`
6. Connect and find port of ESP8266 dev board  
  `arduino-cli board list`
7. Upload  
   `arduino-cli upload -p <PORT> --fqbn deauther:esp8266:d1_mini`  
   or  
   `esptool.py --port <PORT> -b 115200 write_flash 0 <PATH_TO_BIN_FILE>`

## Debug Exceptions using Arduino IDE

1. Install [EspExceptionDecoder](https://github.com/me-no-dev/EspExceptionDecoder) by [downloading](https://github.com/me-no-dev/EspExceptionDecoder/releases/download/1.1.0/EspExceptionDecoder-1.1.0.zip) and extracting it to `<home_dir>/Arduino/tools/`  
  The resulting structure should look like this `<home_dir>/Arduino/tools/EspExceptionDecoder/tool/EspExceptionDecoder.jar`  
2. Open Arduino IDE and press compile or upload
3. Copy stack trace output
4. In Arduino IDE open `Tools` -> `ESP Exception Decoder`
5. Paste strack trace output

## Usage

We recommend using the [Huhnitor](https://github.com/spacehuhntech/huhnitor) for an easy way of interacting with the Deauther serial interface.  
But you can of course use any other serial terminal with 115200baud too.  

## License

This software is licensed under the MIT License. See the [license file](LICENSE) for details.  
