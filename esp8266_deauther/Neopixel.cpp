#include "Neopixel.h"

Neopixel::Neopixel() {

}

// setup pins
void Neopixel::setup(){
  if(!settings.getLedEnabled()) return;

  // ===== adjustable ===== //
  strip.setBrightness(100);
  strip.begin();
  strip.show();
  // ====================== //
  
  setMode(LED_MODE_OFF, true);
}

void Neopixel::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, bool output) {
  // debug output
  if (output){
    char s[30];
    sprintf(s,str(L_OUTPUT_A).c_str(), r, g, b);
    prnt(String(s));
  }
  
  // ===== adjustable ===== //
  if (brightness > 0 && brightness < 100) {
    if (r > 0 && brightness < 100) r = r * brightness / 100;
    if (g > 0 && brightness < 100) g = g * brightness / 100;
    if (b > 0 && brightness < 100) b = b * brightness / 100;
  }

  for(uint16_t i=0;i<LED_NEOPIXEL_NUM;i++){
    strip.setPixelColor(i,strip.Color(r,g,b,brightness));
  }
  strip.show();
  // ====================== //

  // debug output
  if (output){
    char s[30];
    sprintf(s,str(L_OUTPUT_B).c_str(), r?255:0, g?255:0, b?255:0);
    prnt(String(s));
  }
}

// customize color codes for different LED modes
void Neopixel::setMode(uint8_t mode, bool force) {
  // ===== adjustable ===== //
  if (mode != Neopixel::mode || force) {
    Neopixel::mode = mode;
    switch (mode) {
      case LED_MODE_OFF:
        setColor(0, 0, 0, LED_MODE_BRIGHTNESS, false); 
        break;
      case LED_MODE_SCAN:
        if(LED_DYNAMIC_BRIGHTNESS) setColor(0, 0, 255, (uint8_t)(scan.getScaleFactor(100)*scan.getPacketRate()), false); // change color depending on packet rate 
        else  setColor(0, 0, 255, LED_MODE_BRIGHTNESS, false); 
        break;
      case LED_MODE_ATTACK:
        setColor(255, 255, 0, LED_MODE_BRIGHTNESS, false); 
        break;
      case LED_MODE_DEAUTH:
        if(LED_DYNAMIC_BRIGHTNESS) setColor(255, 0, 0, scan.deauths > 255 ? 255 : scan.deauths , false); // brightness depending on how many deauths/s 
        else setColor(255, 0, 0, LED_MODE_BRIGHTNESS, false); 
        break;
      case LED_MODE_IDLE:
        setColor(0, 255, 0, LED_MODE_BRIGHTNESS, false); 
        break;
    }
  }
  // ====================== //
}

void Neopixel::tempEnable() {
  tempEnabled = true;
  prntln(L_ENABLED);
}

void Neopixel::tempDisable() {
  tempEnabled = false;
  prntln(L_DISABLED);
}

bool Neopixel::getTempEnabled() {
  return tempEnabled;
}

void Neopixel::setColor(uint8_t r, uint8_t g, uint8_t b) {
  setColor(r, g, b, 100, true);
}

void Neopixel::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
  setColor(r, g, b, brightness, true);
}

void Neopixel::update() {
  if (!tempEnabled) return;
  if (!settings.getLedEnabled() && tempEnabled) tempDisable();
  
  if (scan.isScanning() && scan.deauths < settings.getMinDeauths()) setMode(LED_MODE_SCAN, false);
  else if (scan.deauths >= settings.getMinDeauths()) setMode(LED_MODE_DEAUTH, false);
  else if (attack.isRunning()) setMode(LED_MODE_ATTACK, false);
  else setMode(LED_MODE_IDLE, false);
}



