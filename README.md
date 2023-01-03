# ESP8266 Deauther Version 2

<p align="center">
  <img alt="PICTURE logo" src="https://raw.githubusercontent.com/wiki/spacehuhn/esp8266_deauther/img/deauther_logo.png" width="200">
  <br>
  <b>Scan for WiFi devices, block selected connections, create dozens of networks and confuse WiFi scanners!</b>
  <br>
  <br>
</p>

## Difference to Version 3

[Version 3](https://github.com/SpacehuhnTech/esp8266_deauther/tree/v3) is in development. It is stable to use, but it is very different.  
It is command line based, which allows it to offer not just more features, but make them more customizable.  

| Feature | Version 2 | Version 3 |
| ------- | --------- | --------- |
| Web Interface | ✅ | |
| Display support | ✅ | |
| Serial Command Line | ✅ | ✅ |
| Scanner | ✅ | ✅ |
| Deauth attack | ✅ | ✅ |
| Beacon attack | ✅ | ✅ |
| Probe attack | ✅ | ✅ |
| [Huhnitor](https://github.com/spacehuhntech/huhnitor) support | | ✅ |
| Signal strength scanner | | ✅ |
| Authentication scanner | | ✅ |
| Rogue AP | | ✅ |

## Installation

Please refer to the [installation guide in our wiki](https://github.com/spacehuhntech/esp8266_deauther/wiki/Installation).  
If you're just looking for the .bin files, go to [releases](https://github.com/spacehuhn/esp8266_deauther/releases).  

## About this project
This software allows you to easily perform a variety of actions to test 802.11 wireless networks by using an inexpensive ESP8266 WiFi SoC (System On A Chip).  

The main feature, the deauthentication attack, is used to disconnect devices from their WiFi network.  
No one seems to care about this huge vulnerability in the official 802.11 WiFi standard, so I took action and enabled everyone who has less than 10 USD to spare to recreate this project.  
I hope it raises more attention on the issue. In 2009 the WiFi Alliance actually fixed the problem (see [802.11w](https://en.wikipedia.org/wiki/IEEE_802.11w-2009)), but only a few companies implemented it into their devices and software.  
To effectively prevent a deauthentication attack, both client and access point must support the 802.11w standard with protected management frames (PMF).  
While most client devices seem to support it when the access point forces it, basically no WiFi access point has it enabled.  

Feel free to test your hardware out, annoy these companies with the problem, share this project and push for a fix!
This project is also a great way to learn more about WiFi, micro controllers, Arduino, hacking and electronics/programming in general.  
**But please use this tool responsibly and do not use it against others without their permission!**

## WiFi Jammer

Many refer to this project as a WiFi jammer. This is problematic, because this firmware is **not** turning your ESP8266 into a radio or frequency jammer. But this is how most people imagine it without further explaination.  

So if you like to learn more about the difference, we made a whole video about it:  
[![Deauther or Jammer: What's the difference?](https://img.youtube.com/vi/gOyfD44A7rE/0.jpg)](https://www.youtube.com/watch?v=gOyfD44A7rE)

## Supported Development Boards

This firmware can run on any **ESP8266**, not however on the ESP32!  
The most popular development boards, the `NodeMCU` and `Wemos d1 mini`, work great for this project.  

If you like to support our work, you can buy get a boards made with this project in mind.  
They come preflashed with the firmware, so you can start right away.  
You can chose between:

**[Spacehuhn Store](https://spacehuhn.store)**:  
Our very own store where all revenue goes directly to us and towards the development of new and exciting projects :)  
We manufacture and ship from Germany.  

**[Maltronics](https://maltronics.com/)**:  
We partnered with Maltronics to bring you high quality hacking gadgets.  
They are based in the UK.  

**[DSTIKE](https://dstike.com/)**:  
DSTIKE was our first partner. They manufactured the first official deauther boards.  
They offer a wide range of deauthers in a variety of form factors, including a smart watch.  
They are based in China, but their products are available through a variety of stores:  
- [Tindie](https://tindie.com/stores/lspoplove)  
- [AliExpress](https://dstike.aliexpress.com/store/2996024)  
- [Taobao](https://shop135375846.taobao.com)  

## Disclaimer

This project is a proof of concept for testing and educational purposes.  
Neither the ESP8266, nor its SDK was meant or built for such purposes. Bugs can occur!  

Use it only against your own networks and devices!  
Please check the legal regulations in your country before using it.  
We don't take any responsibility for what you do with this program.  

## Acknowledgements

A huge thanks to:  

- [@deantonious](http://github.com/deantonious)
- [@jLynx](https://github.com/jLynx)
- [@lspoplove](https://github.com/lspoplove)
- [@schinfo](https://github.com/schinfo)
- [@tobozo](https://github.com/tobozo)
- [@xdavidhu](https://github.com/xdavidhu)
- [@PwnKitteh](https://github.com/PwnKitteh)

for helping out with various things regarding this project and keeping it alive!  
Also thanks to everyone working on the libraries used for this project:  

- [esp8266-oled-ssd1306](https://github.com/ThingPulse/esp8266-oled-ssd1306)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [Adafruit_DotStar](https://github.com/adafruit/Adafruit_DotStar)
- [Adafruit_NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)
- [DS3231](https://github.com/NorthernWidget/DS3231)
- [my92xx](https://github.com/xoseperez/my92xx)

We also thank Espressif and their community for this awesome chip and all the software and hardware projects around it and the countless tutorials you can find online!  

## License 

This software is licensed under the MIT License. See the [license file](LICENSE) for details.  
