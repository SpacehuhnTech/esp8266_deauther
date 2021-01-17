# Settings

**Overview**
  - [`version`](#version)
  - [`ssid`](#ssid)
  - [`password`](#password)
  - [`channel`](#channel)
  - [`hidden`](#hidden)
  - [`captivePortal`](#captiveportal)
  - [`lang`](#lang)
  - [`autosave`](#autosave)
  - [`autosaveTime`](#autosave-time)
  - [`displayInterface`](#display-interface)
  - [`displayTimeout`](#displaytimeout)
  - [`serialInterface`](#serial-interface)
  - [`serialEcho`](#serial-echo)
  - [`webInterface`](#web-interface)
  - [`webSpiffs`](#web-spiffs)
  - [`ledEnabled`](#ledEnabled)
  - [`maxCh`](#max-ch)
  - [`macAP`](#macap)
  - [`macSt`](#macst)
  - [`chTime`](#ch-time)
  - [`minDeauths`](#mindeauths)
  - [`attackTimeout`](#attacktimeout)
  - [`deauthsPerTarget`](#deauths-per-target)
  - [`deauthReason`](#deauth-reason)
  - [`beaconChannel`](#beacon-channel)
  - [`beaconInterval`](#beacon-interval)
  - [`randomTx`](#randomtx)
  - [`probesPerSSID`](#probesperssid)
  
## VERSION
`String version = VERSION;`  
Version number, i.e. `v2.0`.  
**PLEASE NOTE** that this setting can only be changed in the source code.  

## SSID
`String ssid = "pwned";`  
SSID of access point used for the web interface (if enabled).  
The length must be between 1 and 31 characters.  

## PASSWORD
`String password = "deauther";`  
Password of access point used for the web interface (if enabled).  
The length must be between 8 and 31 characters.  

## CHANNEL
`uint8_t channel = 1;`  
Default WiFi channel that is used when starting.  

## HIDDEN
`bool hidden = false`  
Hides the access point that is used for the web interface (if enabled).   

## CAPTIVEPORTAL
`bool captivePortal = true;`  
Enables captive portal for access point (if enabled).  

## LANG
`String lang = "en";`  
Default language for the web interface.  
Be sure the language file exists!  

## AUTOSAVE
`bool autosave = true;`  
Enables automatic saving of SSIDs, device names and settings.  

## AUTOSAVE-TIME
`uint32_t autosaveTime = 30000;`  
Time interval for automatic saving in milliseconds.  

## DISPLAY-INTERFACE
`bool displayInterface = false;`  
Enables display interface.  

## DISPLAYTIMEOUT
`uint32_t displayTimeout = 600`  
Time in seconds after which the display turns off when inactive.  
To disable the display timeout, set it to 0.  

## SERIAL-INTERFACE
`bool serialInterface = true;`  
Enables serial interface.  

## SERIAL-ECHO
`bool serialEcho = true`  
Enables echo for each incoming message over serial.  

## WEB-INTERFACE
`bool webInterface = false;`  
Enables web interface.  

## WEB-SPIFFS
`bool webSpiffs = false`  
Enables SPIFFS for all web files.  
Can lead to longer loading times but it nice if you need to edit the web files regularly.  

## LEDENABLED
`bool ledEnabled = true`  
Enables the (RGB) LED feature.  

## MAX-CH
`uint8_t maxCh = 13;`  
Max channel to scan on.  
US = 11, EU = 13, Japan = 14.  
For more information click [here](https://en.wikipedia.org/wiki/List_of_WLAN_channels).  

## MACAP
`uint8_t* macAP;`  
Mac address used for the access point mode.  
Please note that the mac address will only replace the internal mac address when the accesspoint mode is enabled.  
You can set a random mac address with `set macap random`.  

## MACST
`uint8_t* macSt;`  
Mac address used for the station mode.  
Please note that the mac address will only replace the internal mac address when the station mode is enabled.  
You can set a random mac address with `set macst random`.  

## CH-TIME
`uint16_t chTime = 384;`  
Time for scanning one channel before going to the next in milliseconds (only if channel hopping is enabled).  

## MINDEAUTHS
`uint16_t minDeauths = 3`  
Minimum number of deauthentication frames when scanning to change the LED to deauth mode.  

## ATTACKTIMEOUT
`uint32_t attackTimeout = 600`  
After what amount of time (in seconds) the attack will stop automatically.  
Set it to 0 to disable it.  

## DEAUTHS-PER-TARGET
`uint16_t deauthsPerTarget = 10;`  
How many deauthentication and disassociation frames are sent out for each target.  

## DEAUTH-REASON
`uint8_t deauthReason = 1;`  
The [reason code](https://www.cisco.com/assets/sol/sb/WAP371_Emulators/WAP371_Emulator_v1-0-1-5/help/Apx_ReasonCodes2.html) that is sent with the deauth frames to tell the target device why the connection will be closed.  

## BEACON-CHANNEL
`bool beaconChannel = false;`  
If enabled, will sent all beacon and probe frames on different channels when running a beacon attack.  

## BEACON-INTERVAL
`bool beaconInterval = false;`  
If set true, beacons will be sent out every second. If set to false, the interval will be 100ms. 
A longer interval means more stability and less spamming of packets, but it could take longer 
until the clients find the ssids when scanning.  

## RANDOMTX
`bool randomTX = false`  
Enables randomized transmission power for sending out beacon and probe request frames.  

## PROBESPERSSID
`uint8_t probesPerSSID = 1`  
How many probe request frames are sent for each SSID.     
