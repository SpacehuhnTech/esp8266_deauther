#ifndef __SDUPDATER_H
#define __SDUPDATER_H

#include <SPI.h>
// Depending on the SDK version this may require a small patch in "FS.h", see
//     https://github.com/esp8266/Arduino/search?utf8=%E2%9C%93&q=FS_NO_GLOBALS
// If you also get namespace conflicts you may need to patch you app too:
//     https://github.com/esp8266/Arduino/issues/2281#issuecomment-258706478
#define FS_NO_GLOBALS
#include <SD.h>

static String MENU_BIN = "/MENU.BIN";


class SDUpdater {
  public: 
    void run();
    void updateFromSD(String fileName = MENU_BIN );

  private:
    void performUpdate(Stream &updateSource, size_t updateSize, String fileName);
};


#endif

