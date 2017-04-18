# ESP8266 Deauther
Deauthentication attack and other hacks using an ESP8266.

![esp8266 deauther with a smartphone](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/smartphone_esp_1.jpg)

## Contents
- [Introduction](#introduction)
  - [What it is](#what-it-is)
  - [How it works](#how-it-works)
  - [What an ESP8266 is](#what-an-esp8266-is)
  - [How to protect against it](#how-to-protect-against-it)
- [Disclaimer](#disclaimer)
- [Installation](#installation)
  - [Uploading the bin files](#uploading-the-bin-files)  
  - [Compiling the source with Arduino](#compiling-the-source-with-arduino)
  - [Adding OLED display](#adding-oled-display)
- [How to use it](#how-to-use-it)
- [FAQ](#faq)
- [Videos](#videos)
- [License](#license)
- [Sources and additional links](#sources-and-additional-links)

## Introduction ##

### What it is

Basically it’s a device which performs a [deauth attack](https://en.wikipedia.org/wiki/Wi-Fi_deauthentication_attack).  
You select the clients you want to disconnect from their network and start the attack. As long as the attack is running, the
selected devices are unable to connect to their network.  
Other attacks also have been implemented, such as beacon or probe request flooding.  

### How it works

The 802.11 Wi-Fi protocol contains a so called [deauthentication frame](https://mrncciew.com/2014/10/11/802-11-mgmt-deauth-disassociation-frames/). It is used to disconnect clients safely from a wireless
network.

Because these management packets are unencrypted, you just need the mac address of the Wi-Fi router and of the client device which you want to disconnect from the network. You don’t need to be in the network or know the password, it’s enough to be in its range.

### What an ESP8266 is

The [ESP8266](https://de.wikipedia.org/wiki/ESP8266) is a cheap micro controller with built-in Wi-Fi. It contains a powerfull 160 MHz processor and it can be programmed using [Arduino](https://www.arduino.cc/en/Main/Software).  

You can buy these chips for under $2 from China!

### How to protect against it

With [802.11w-2009](https://en.wikipedia.org/wiki/IEEE_802.11w-2009) Wi-Fi got an update to encrypt management frames.
So make sure your router is up to date and has management frame protection enabled. But note that your client device needs to 
support it too, both ends need to have it enabled!

The only problem is that most devices don’t use it. I tested it with different Wi-Fi networks and devices, it worked every time! It seems that even newer devices which support frame protection don’t use it by default.

## Disclaimer

Use it only for testing purposes on your own devices!  
I don't take any responsibility for what you do with this program.  

Please check the legal regulations in your country before using it. Jamming transmitters are illegal in most countries 
and this device can fall into the same category (even if it’s technically not the same).

My intention with this project is to draw attention to this issue. 
This attack shows how vulnerable the 802.11 Wi-Fi standard is and that it has to be fixed.
**A solution is already there, why don’t we use it?**

## Installation

The only thing you will need is a computer and an ESP8266.  

I recommend you to buy a USB breakout/developer board, because they have 4Mb flash and are very simple to use.
It doesn’t matter which board you use, as long as it has an ESP8266 on it.  

You have 2 choices here. Uploading the bin files is easier but not as good for debugging, so keep that in mind in case you want to open an new issue. 
**YOU ONLY NEED TO DO ONE OF THE INSTALLATION METHODS!**

### Uploading the bin files  

**Note:** the 512kb version won't have the full MAC vendor list.  
The NodeMCU and every other board which uses the ESP-12 has 4mb flash on it.

**0** Download the current release from [here](https://github.com/spacehuhn/esp8266_deauther/releases)  

**1** Upload using the ESP8266 flash tool of your choice. I recommend using the [nodemcu-flasher](https://github.com/nodemcu/nodemcu-flasher). If this doesn't work you can also use the official [esptool](https://github.com/espressif/esptool) from espressif.

**That's all! :)**

Make sure you select the right com-port, the right upload size of your ESP8266 and the right bin file.  

If flashing the bin files with a flash tool is not working, try flashing the esp8266 with the Arduino IDE as shown below.

### Compiling the source with Arduino

**0** Download the source code of this project.

**1** Install [Arduino](https://www.arduino.cc/en/Main/Software) and open it.

**2** Go to `File` > `Preferences`

**3** Add `http://arduino.esp8266.com/stable/package_esp8266com_index.json` to the Additional Boards Manager URLs. (source: https://github.com/esp8266/Arduino)

**4** Go to `Tools` > `Board` > `Boards Manager`

**5** Type in `esp8266`

**6** Select version `2.0.0` and click on `Install` (**must be version 2.0.0!**)

![screenshot of arduino, selecting the right version](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/arduino_screenshot_1.JPG)

**7** Go to `File` > `Preferences`

**8** Open the folder path under `More preferences can be edited directly in the file`

![screenshot of arduino, opening folder path](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/arduino_screenshot_2.JPG)

**9** Go to `packages` > `esp8266` > `hardware` > `esp8266` > `2.0.0` > `tools` > `sdk` > `include`

**10** Open `user_interface.h` with a text editor

**11** Scroll down and before `#endif` add following lines:

`typedef void (*freedom_outside_cb_t)(uint8 status);`  
`int wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);`  
`void wifi_unregister_send_pkt_freedom_cb(void);`  
`int wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);`  

![screenshot of notepad, copy paste the right code](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/notepad_screenshot_1.JPG)

**don't forget to save!**

**12** Go to the SDK_fix folder of this project

**13** Copy ESP8266Wi-Fi.cpp and ESP8266Wi-Fi.h

**14** Paste these files here `packages` > `esp8266` > `hardware` > `esp8266` > `2.0.0` > `libraries` > `ESP8266WiFi` > `src`

**15** Open `esp8266_deauther` > `esp8266_deauther.ino` in Arduino

**16** Select your ESP8266 board at `Tools` > `Board` and the right port at `Tools` > `Port`  
If no port shows up you may have to reinstall the drivers.

**17** Depending on your board you may have to adjust the `Tools` > `Board` > `Flash Frequency` and the `Tools` > `Board` > `Flash Size`. In my case i had to use a `80MHz` Flash Frequency, and a `4M (1M SPIFFS)` Flash Size

**18** Upload!

**Note:** If you use a 512kb version of the ESP8266, you need to comment out a part of the mac vendor list in data.h.

**Your ESP8266 Deauther is now ready!**


### Adding OLED display

![image of the esp8266 deauther with an OLED and three buttons](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/esp8266_with_oled.jpg)

**0** Follow the steps [above](#compiling-the-source-with-arduino) to get your Arduino environment ready.

**1** Install this OLED driver library: https://github.com/squix78/esp8266-oled-ssd1306

**2** Custimize the code for your wiring.  
		In `esp8266_deauther.ino` uncomment `#define USE_DISPLAY`.  
		Then scroll down and custimize these lines depending on your setup.  
		I used a Wemos d1 mini with a SSD1306 128x64 OLED and 3 push buttons.  

		  //include the library you need
		  #include "SSD1306.h"
		  //#include "SH1106.h"

		  //button pins
		  #define upBtn D6
		  #define downBtn D7
		  #define selectBtn D5

		  #define buttonDelay 180 //delay in ms
		  
		  //render settings
		  #define fontSize 8
		  #define rowsPerSite 8

		  //create display(Adr, SDA-pin, SCL-pin)
		  SSD1306 display(0x3c, D2, D1);
		  //SH1106 display(0x3c, D2, D1);

## How to use it

First start your ESP8266 by giving it power.  

You can use your smartphone if you have a USB OTG cable.
![esp8266 deauther with a smartphone](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/smartphone_esp_2.jpg)

Scan for Wi-Fi networks and connect to `pwned`. The password is `deauther`.  
Once connected, you can open up your browser and go to `192.168.4.1`.  

You can now scan for networks...
![webinterface AP scanner](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/web_screenshot_1.JPG)

scan for client devices... 
![webinterface client scanner](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/web_screenshot_2.JPG)

Note: While scanning the ESP8266 will shut down its access point, so you may have to go to your settings and reconnect to the Wi-Fi network manually.

...and start different attacks.
![webinterface attack menu](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/web_screenshot_3.JPG)

Happy hacking :)

## FAQ

**Could it auto-deauth all APs in the range?**

Yes, but I will not implement this 'feature' for ethical and legal reasons.

**Can it sniff handshakes?**

The ESP8266 has a promiscuous mode in which you can sniff packets, but handshake packets are dropped and there is no other way to get them with the functions provided by the SDK.  
Maybe someone will find a way around this barrier in the future.

**espcomm_sync failed/espcomm_open when uploading**

The ESP upload tool can't communicate with the chip, make sure the right port is selected!  
You can also try out different USB ports and cables.  
If this doesn't solve it you may have to install USB drivers.  
Which drivers you need depends on the board, most boards use a cp2102 or ch340.

**AP scan doesn't work**

There is a reported issue on this: https://github.com/spacehuhn/esp8266_deauther/issues/5  
Try switching the browser or opening the website with another device.   

**Deauth attack won't work**

If you see 0 pkts/s on the website then you've made a mistake. Check that have followed the the installation steps correctly and that the right SDK installed, it must be version 2.0.0!
If it can send packets but your target doesn't loose its connection, then the Wi-Fi router either uses [802.11w](#how-to-protect-against-it) and it's protected against such attacks, or it communicates on the 5GHz band, which the ESP8266 doesn't support because of its 2.4GHz antenna.

### If you have other questions or problems with the ESP8266 you can also check out the official [community forum](http://www.esp8266.com/).

## Videos
  
[![Cheap Wi-Fi 'Jammer' Device | NodeMCU](https://img.youtube.com/vi/oQQhBdCQOTM/0.jpg)](https://www.youtube.com/watch?v=oQQhBdCQOTM)
  
[![Wifi 'Jammer' Device V1.1 | Setup Tutorial](https://img.youtube.com/vi/r5aoV5AolNo/0.jpg)](https://www.youtube.com/watch?v=r5aoV5AolNo)
  
[![WiFi Jamming Tutorial "Deauthing Made Simple" ](https://img.youtube.com/vi/SswI-J-M2SE/0.jpg)](https://www.youtube.com/watch?v=SswI-J-M2SE)
  
[![NodeMCU ESP8266 Tutorial 02: WiFi Hack with ESP8266 (NodeMCU WiFi Jammer)](https://img.youtube.com/vi/MOscKnm8IcY/0.jpg)](https://www.youtube.com/watch?v=MOscKnm8IcY)

## License

This project is licensed under the MIT License - see the [license file](LICENSE) file for details.

**The License file must be included in any redistributed version of this program!**

## Sources and additional links

deauth attack: https://en.wikipedia.org/wiki/Wi-Fi_deauthentication_attack

deauth frame: https://mrncciew.com/2014/10/11/802-11-mgmt-deauth-disassociation-frames/

ESP8266: 
* https://de.wikipedia.org/wiki/ESP8266
* https://espressif.com/en/products/hardware/esp8266ex/overview

packet injection with ESP8266: 
* http://hackaday.com/2016/01/14/inject-packets-with-an-esp8266/
* http://bbs.espressif.com/viewtopic.php?f=7&t=1357&p=10205&hilit=Wi-Fi_pkt_freedom#p10205
* https://github.com/pulkin/esp8266-injection-example

802.11w-2009: https://en.wikipedia.org/wiki/IEEE_802.11w-2009

Wi-Fi_send_pkt_freedom function limitations: http://esp32.com/viewtopic.php?f=13&t=586&p=2648&hilit=Wi-Fi_send_pkt_freedom#p2648
