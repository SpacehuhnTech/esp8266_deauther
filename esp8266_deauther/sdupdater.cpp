// based on https://github.com/tobozo/ESP8266SDUpdater
#include "sdupdater.h"

// perform the actual update from a given stream
void SDUpdater::performUpdate(Stream &updateSource, size_t updateSize, String fileName) {
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


void SDUpdater::updateFromSD(String fileName) {
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
  #ifndef HAS_SD
    Serial.println("Cowardly refusing to load a SD menu as this build has no SD card defined. See A_Config.h for more details");
    return;
  #endif
  
  #ifdef SD_CS
    SD.begin(SD_CS);
  #else
   // none
   SD.begin();
  #endif
  
  Serial.println("Will load MENU");
  updateFromSD(MENU_BIN);
  ESP.restart();
}
