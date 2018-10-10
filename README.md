# ESP8266 Deauther 2.0

<p align="center"><img alt="PICTURE logo" src="https://raw.githubusercontent.com/wiki/spacehuhn/esp8266_deauther/img/deauther_logo.png" width="200"></p>

<p align="center">
🐦 <a href="https://twitter.com/spacehuhn">Twitter</a>
| 📺 <a href="https://www.youtube.com/channel/UCFmjA6dnjv-phqrFACyI8tw">YouTube</a>
| 🌍 <a href="https://spacehuhn.de">spacehuhn.de</a><br>
<br>
<b>Scan for WiFi devices, block selected connections, create dozens of networks and confuse WiFi scanners!<br><br>
Click <a href="https://github.com/spacehuhn/esp8266_deauther/wiki/Installation">here</a> for the installation tutorial.<br>
  Click <a href="https://github.com/spacehuhn/esp8266_deauther/releases">here</a> for the .bin files.<br><br>
Support the development of this project by purchasing one of the <a href="https://github.com/spacehuhn/esp8266_deauther/wiki/Supported-Devices">official deauther boards</a>.<br/>
Or become a patron on <a href="https://patreon.com/spacehuhn" target="_blank">patreon.com/spacehuhn</a>.<br>
Also available: <a href="https://www.tindie.com/products/Spacehuhn/spacehuhn-stickers/">Stickers</a></b>!
</p>

## What is New
[Here is a quick video about what is new](https://youtu.be/6oRmm3xfp6U)  
Version 2.0:
- Completly rewritten code base for better performance and later enhancements
- Custom Deauther SDK for easy compiling using Arduino
- New serial command line interface to control and debug the program
- New display UI with a lot of new functions
- Improved web interface with multi-language support
- Improved scanning for access points and stations (+ continuous scanning mode)
- Save and select device names for both scanning and attacking
- Save up to 60 SSIDs and 25 devices in one list (you can create, load and save multiple lists)
- Added [PacketMonitor](https://github.com/spacehuhn/PacketMonitor) to display UI
- Deauth detection when scanning
- RGB LED support for a quick indication what the device is doing (attacking, scanning, ...)
- Better documentation on the new [wiki](https://github.com/spacehuhn/esp8266_deauther/wiki)

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

The difference between deauthing and jamming: [click me](https://github.com/spacehuhn/esp8266_deauther/wiki/FAQ#difference-between-jammer-and-deauther)

## Official Deauther Boards

![PICTURE DSTIKE Deauther OLED Board](https://raw.githubusercontent.com/wiki/spacehuhn/esp8266_deauther/img/DSTIKE_Deauther_Board.jpg)

If you want to support the development of this project, you can buy one of the official boards by DSTIKE (Travis Lin) on following sites:  
- [Tindie](https://tindie.com/stores/lspoplove)  
- [AliExpress](https://dstike.aliexpress.com/store/2996024)  
- [Taobao](https://shop135375846.taobao.com)  

Those boards are optimized for this project, ready to use and come preflashed with the Deauther software!  
For more details visit the [Wiki](https://github.com/spacehuhn/esp8266_deauther/wiki) under [Supported Devices](https://github.com/spacehuhn/esp8266_deauther/wiki/Supported-Devices).  

## Disclaimer
This project is a proof of concept for testing and educational purposes.  
Neither the ESP8266, nor its SDK was meant or built for such purposes. Bugs can occur!  

Use it only against your own networks and devices!  
Please check the legal regulations in your country before using it.  
I don't take any responsibility for what you do with this program.  

It is **not a frequency jammer** as claimed falsely by many people. Its attack, its method and how to protect against it is described above. It uses valid Wi-Fi frames described in the IEEE 802.11 standard and doesn't block or disrupt any frequencies.  

This project is meant to draw more attention on this issue.  
The [deauthentication](https://en.wikipedia.org/wiki/Wi-Fi_deauthentication_attack) attack shows how vulnerable the 802.11 Wi-Fi standard is and that it has to be fixed.  
A solution is already there, why don't we use it?

**Please don't refer to this project as "jammer", that totally undermines the real purpose of this project!**
If you do, it only proves that you didn't understand anything of what this project stands for. Publishing content about this without a proper explanation shows that you only do it for the clicks, fame and/or money and have no respect for intellectual property, the community behind it and the fight for a better WiFi standard!  

## Getting Started

Visit our new [Wiki](https://github.com/spacehuhn/esp8266_deauther/wiki) on how to recreate this project and use it.  
Happy Hacking!

## Credits
A huge thanks to:
- [@deantonious](http://github.com/deantonious)
- [@jLynx](https://github.com/jLynx)
- [@lspoplove](https://github.com/lspoplove)
- [@schinfo](https://github.com/schinfo)
- [@tobozo](https://github.com/tobozo)
- [@xdavidhu](https://github.com/xdavidhu)
- [@PwnKitteh](https://github.com/PwnKitteh)

for helping out with various things regarding this project and keeping it alive!  

I also want to thank Espressif and their community for this awesome chip and all the software and hardware projects around it and the countless tutorials you can find online!  

Shoutout to everyone working on the libraries used for this project:
- [esp8266-oled-ssd1306](https://github.com/ThingPulse/esp8266-oled-ssd1306)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [LinkedList](https://github.com/ivanseidel/LinkedList)

Also thanks to everyone that supports this project by [donating](http://spacehuhn.de/donate), being my [patron](http://patreon.com/spacehuhn) or buying one of the [official Deauther boards](https://www.tindie.com/stores/lspoplove) from DSTIKE.  

## License 

This software is licensed under the MIT License. See the [license file](LICENSE) for details.  
