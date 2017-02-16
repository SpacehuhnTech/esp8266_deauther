# ESP8266 Deauther
Build your own WiFi jammer with an ESP8266.

![esp8266 deauther with a smartphone](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/smartphone_esp_1.jpg)

## Contents
- [Introduction](#introduction)
  - [What it is](#what-it-is)
  - [How it works](#how-it-works)
  - [What an ESP8266 is](#what-an-esp8266-is)
  - [How to protect against it](#how-to-protect-against-it)
- [Disclaimer](#disclaimer)
- [Installation](#installation)
- [How to use it](#how-to-use-it)
- [FAQ](#faq)
- [License](#license)
- [Sources and additional links](#sources-and-additional-links)

## Introduction ##

### What it is

Basically it’s a device which performs a [deauth attack](https://en.wikipedia.org/wiki/Wi-Fi_deauthentication_attack).  
You select the clients you want to disconnect from their network and start the attack. As long as the attack is running, the
selected devices are unable to connect to their network.

### How it works

The 802.11 WiFi protocol contains a so called [deauthentication frame](https://mrncciew.com/2014/10/11/802-11-mgmt-deauth-disassociation-frames/). It is used to disconnect clients safely from a wireless
network.

Because these packets are unencrypted, you just need the mac address of the WiFi router and of the client device which you want to disconnect from the network. You don’t need to be in the network or know the password, it’s enough to be in its range.

### What an ESP8266 is

The [ESP8266](https://de.wikipedia.org/wiki/ESP8266) is a very cheap micro controller with build in WiFi. It contains a powerfull 160 MHz processor and you can program it
with the [Arduino IDE](https://www.arduino.cc/en/Main/Software). This makes it perfect for this project.

You can buy these chips for under $2 from China!

### How to protect against it

With [802.11w-2009](https://en.wikipedia.org/wiki/IEEE_802.11w-2009) WiFi got an update to encrypt management frames.
So make sure your router is up to date and has management frame protection enabled. But be sure that your client device
supports it too, both ends need to have it enabled!

The only problem is that most devices don’t use it. I tested it with different WiFi networks and devices, it worked every time! It seems that even newer devices which support frame protection don’t use it by default.

## Disclaimer

Use it only for testing purposes on your own devices!

Please check the legal regulations in your country before using it. Jamming transmitters are illegal in most countries 
and this device can fall into the same category (even if it’s technically not the same).

My intention with this project is to draw attention to this issue. 
This attack shows how vulnerable the 802.11 WiFi standard is and that it has to be fixed.
**A solution is already there, why don’t we use it?**

## Installation

The only thing you will need is a computer and an ESP8266 with at least 1Mb of flash memory.

I recommend you to buy a USB breakout/developer board, because they have 4Mb flash and are very simple to use.
It doesn’t matter which board you use, as long as it has an ESP8266 on it.  
(If you using an ESP-01 with just 512kb, you can comment out the mac vendor list in data.h.)

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

**12** Download and open `esp8266_deauther` > `esp8266_deauther.ino` in Arduino

**13** Select your ESP8266 board at `Tools` > `Board` and the right port at `Tools` > `Port`  
If no port shows up you may have to reinstall the drivers.

**14** Upload!

**Your ESP8266 Deauther is now ready!**

## How to use it

First start your ESP8266 by giving it power.  

You can use your smartphone if you have a USB OTG cable.
![esp8266 deauther with a smartphone](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/smartphone_esp_2.jpg)

Scan for WiFi networks and connect to `pwned`. The password is `deauther`.  
Once connected, you can open up your browser and go to `192.168.4.1`.  

You can now scan for networks...
![webinterface AP scanner](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/web_screenshot_1.JPG)

scan for client devices... 
![webinterface client scanner](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/web_screenshot_2.JPG)

Note: While scanning the ESP8266 will shut down its access point, so you may have to go to your settings and reconnect to the WiFi network manually.

...and start different attacks.
![webinterface attack menu](https://raw.githubusercontent.com/spacehuhn/esp8266_deauther/master/screenshots/web_screenshot_3.JPG)

Happy hacking :)

## FAQ

**Could it deauth multiple APs in the range?**

It definitely could! But I will not implement this 'feature' for ethical and legal reasons.

**Can it sniff handshakes?**

The ESP8266 has a promiscuous mode in which you can sniff nearly all packets, but handshake packets are dropped and there is no other way to get them with the functions provided by the SDK.  
Maybe someone will find a way around this barrier but I wasn't able to.

**espcomm_sync failed/espcomm_open when uploading**

The ESP upload tool can't communicate with the chip, make sure the right port is selected!  
You can also try out different USB ports and cables.  
If this doesn't solve it you may have to install USB drivers.  
Which drivers you need depends on the board, most boards use a cp2102, cp2104 or ch340.

**AP scan doesn't work**

There is a reported issue with the Internet Explorer: https://github.com/spacehuhn/esp8266_deauther/issues/5  
Try out switching the browser or open the website with another device.   

**Deauth attack won't work**

If you see 0 pkts/s on the website you have made a mistake. Check if you have followed the the installation steps correctly and that the right SDK installed, it must be version 2.0.0!  
If it can send packets but your target don't loose its connection then the WiFi router uses [802.11w](#how-to-protect-against-it) and it's protected against such attacks or they communicate via 5 GHz WiFi, which the ESP8266 doesn't support.


###If you have other questions or problems with the ESP8266 you can also check out the official [community forum](http://www.esp8266.com/).


## License

This project is licensed under the MIT License - see the [license file](LICENSE) file for details

## Sources and additional links

deauth attack: https://en.wikipedia.org/wiki/Wi-Fi_deauthentication_attack

deauth frame: https://mrncciew.com/2014/10/11/802-11-mgmt-deauth-disassociation-frames/

ESP8266: 
* https://de.wikipedia.org/wiki/ESP8266
* https://espressif.com/en/products/hardware/esp8266ex/overview

packet injection with ESP8266: 
* http://hackaday.com/2016/01/14/inject-packets-with-an-esp8266/
* http://bbs.espressif.com/viewtopic.php?f=7&t=1357&p=10205&hilit=wifi_pkt_freedom#p10205
* https://github.com/pulkin/esp8266-injection-example

802.11w-2009: https://en.wikipedia.org/wiki/IEEE_802.11w-2009

wifi_send_pkt_freedom function limitations: http://esp32.com/viewtopic.php?f=13&t=586&p=2648&hilit=wifi_send_pkt_freedom#p2648

esp32 esp_wifi_internal function limitations: http://esp32.com/viewtopic.php?f=13&t=586&p=2648&hilit=wifi_send_pkt_freedom#p2648

Videos:  
[![Cheap Wifi 'Jammer' Device | NodeMCU](https://img.youtube.com/vi/oQQhBdCQOTM/0.jpg)](https://www.youtube.com/watch?v=oQQhBdCQOTM)
