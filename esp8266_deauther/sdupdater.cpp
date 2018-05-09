#include "sdupdater.h"
// based on https://github.com/tobozo/ESP8266SDUpdater

// perform the actual update from a given stream
void SDUpdater::performUpdate(Stream &updateSource, size_t updateSize, String fileName) {
   //Update.onProgress(ESP8266SDMenuProgress);
   if (Update.begin(updateSize)) {      
      size_t written = Update.writeStream(updateSource);
      if (written == updateSize) {
         Serial.println("Written : " + String(written) + " successfully");
      } else {
         Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      }
      if (Update.end()) {
         Serial.println("FOTA done!");
         if (Update.isFinished()) {
            Serial.println("Update successfully completed. Rebooting.");
         } else {
            Serial.println("Update not finished? Something went wrong!");
         }
      } else {
         Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }
   } else {
      Serial.println("Not enough space to begin FOTA");
   }
}

// check given FS for valid MENU_BIN and perform update if available
void SDUpdater::updateFromSD(String fileName) {
  SD.begin();
  File root = SD.open("/");

  File updateBin = SD.open(fileName);

  size_t updateSize = updateBin.size();
  if (updateSize > 0) {
    Serial.println("Try to start update");
    performUpdate(updateBin, updateSize, fileName);
  } else {
     Serial.println("Error, file is empty");
  }
  updateBin.close();

}



void SDUpdater::run() {
  /*
   * GPIO  4 SDA
   * GPIO  5 SCK/SCL for i2c
   * GPIO 12 MISO
   * GPIO 13 MOSI
   * GPIO 14 CLK
   * GPIO 15 CS 
   */
  while(!SD.begin()) {
    // TODO: make a more fancy animation
    Serial.println("Insert SD");
    //display.clear();
    //display.display();
    delay(300);
    //display.drawString(10, 20, "INSERT SD");
    //display.display();
  }
  Serial.println("Will load MENU");
  updateFromSD(MENU_BIN);
  ESP.restart();
}
