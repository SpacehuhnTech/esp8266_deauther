#include <ESP8266SDUpdater.h>
#include <EEPROM.h>
#include <FS.h>

SDUpdater sdUpdater;

/*
  Upload this sketch to your ESP8266 to erase 
  - all files in the SPIFFS, 
  - all data in the EEPROM
  - WiFi credentials (SSID, password)

  Also overwrites the previous program with this one (obviously).
*/

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println("STARTING...");
  
  EEPROM.begin(4096);
  Serial.println("EEPROM initialized");
  
  for (int i = 0; i < 4096; ++i){
    EEPROM.write(i,0x00);
  }

  Serial.println("EEPROM cleaned");

  SPIFFS.begin();
  Serial.println("SPIFFS initialized");

  SPIFFS.format();
  Serial.println("SPIFFS cleaned");
  
  ESP.eraseConfig();

  Serial.println("WiFi credentials erased");

  Serial.println("DONE!");

  // reload deauther
  if(SD.begin(/* SD_CS */) && sdUpdater.updateFromSD("/MENU.BIN") ) {
    Serial.println("Will restart and load menu");
    ESP.restart();  
  } else {
    Serial.println("Will reset, now flash the deauther!");
    delay(10000);
    ESP.reset();
  }
  
}

void loop() {
  
}
