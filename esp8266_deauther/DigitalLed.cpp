#include "DigitalLed.h"

DigitalLed::DigitalLed() {

}

// setup pins
void DigitalLed::setup(){
  if(!settings.getLedEnabled()) return;

  // ===== adjustable ===== //
  if(LED_ENABLE_R) pinMode(LED_PIN_R, OUTPUT);
  if(LED_ENABLE_G) pinMode(LED_PIN_G, OUTPUT);
  if(LED_ENABLE_B) pinMode(LED_PIN_B, OUTPUT);
  // ====================== //
  
  setMode(LED_MODE_OFF, true);
}

void DigitalLed::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, bool output) {
  // debug output
  if (output){
    char s[30];
    sprintf(s,str(L_OUTPUT_A).c_str(), r, g, b);
    prnt(String(s));
  }
  
  // ===== adjustable ===== //
  if(LED_CATHODE){
    r = r>0;
    g = g>0;
    b = b>0;
  }else{
    r = !(r>0);
    g = !(g>0);
    b = !(b>0);
  }

  if(LED_ENABLE_R) digitalWrite(LED_PIN_R, r);
  if(LED_ENABLE_G) digitalWrite(LED_PIN_G, g);
  if(LED_ENABLE_B) digitalWrite(LED_PIN_B, b);
  // ====================== //

  // debug output
  if (output){
    char s[30];
    sprintf(s,str(L_OUTPUT_B).c_str(), r?255:0, g?255:0, b?255:0);
    prnt(String(s));
  }
}

// customize color codes for different LED modes
void DigitalLed::setMode(uint8_t mode, bool force) {
  // ===== adjustable ===== //
  if (mode != DigitalLed::mode || force) {
    DigitalLed::mode = mode;
    switch (mode) {
      case LED_MODE_OFF:
        setColor(0, 0, 0, 100, false);
        break;
      case LED_MODE_SCAN:
        setColor(0, 0, 255, 100, false);
        break;
      case LED_MODE_ATTACK:
        setColor(255, 255, 0, 100, false);
        break;
      case LED_MODE_DEAUTH:
        setColor(255, 0, 0, 100, false);
        break;
      case LED_MODE_IDLE:
        setColor(0, 255, 0, 100, false);
        break;
    }
  }
  // ====================== //
}

void DigitalLed::tempEnable() {
  tempEnabled = true;
  prntln(L_ENABLED);
}

void DigitalLed::tempDisable() {
  tempEnabled = false;
  prntln(L_DISABLED);
}

bool DigitalLed::getTempEnabled() {
  return tempEnabled;
}

void DigitalLed::setColor(uint8_t r, uint8_t g, uint8_t b) {
  setColor(r, g, b, 100, true);
}

void DigitalLed::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
  setColor(r, g, b, brightness, true);
}

void DigitalLed::update() {
  if (!tempEnabled) return;
  if (!settings.getLedEnabled() && tempEnabled) tempDisable();
  
  if (scan.isScanning() && scan.deauths < settings.getMinDeauths()) setMode(LED_MODE_SCAN, false);
  else if (scan.deauths >= settings.getMinDeauths()) setMode(LED_MODE_DEAUTH, false);
  else if (attack.isRunning()) setMode(LED_MODE_ATTACK, false);
  else setMode(LED_MODE_IDLE, false);
}



