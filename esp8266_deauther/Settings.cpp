#include "Settings.h"

Settings::Settings() {
  uint8_t tempMAC[6];
  defaultMacAP.set(WiFi.softAPmacAddress(tempMAC));
  if(!defaultMacAP.valid()) defaultMacAP.randomize();
}

void Settings::syncMacInterface(){
  if(debug) Serial.println("Trying to sync the MAC addr with settings");
  if(isSettingsLoaded){
    Mac macToSync;
    if(isMacAPRand){
      macToSync.randomize();
      wifi_set_macaddr(SOFTAP_IF, macToSync._get());
      if(debug) Serial.println("Synced with a random mac addr : " + macToSync.toString());
    }else if(macAP.valid()){
      macToSync = macAP;
      wifi_set_macaddr(SOFTAP_IF, macToSync._get());
      if(debug) Serial.println("Synced with saved mac addr : " + macToSync.toString());
    }else{
      if(debug) Serial.println("Could not sync because of invalid settings !");
    }
  }else{
    if(debug) Serial.println("Could not sync because settings are not loaded !");
  }
}

void Settings::setLedPin(int newLedPin){
  prevLedPin = ledPin;
  if(newLedPin > 0 && newLedPin != prevLedPin){
    ledPin = newLedPin;
    pinMode(ledPin, OUTPUT);
    if(!prevLedPin == 0){
      digitalWrite(ledPin, digitalRead(prevLedPin));
      digitalWrite(prevLedPin, pinStateOff);
      pinMode(prevLedPin, INPUT);
    }else{
      digitalWrite(ledPin, pinStateOff);
    }
  }
}

void Settings::load() {

  if (EEPROM.read(checkNumAdr) != checkNum) {
    reset();
    return;
  }

  ssidLen = EEPROM.read(ssidLenAdr);
  passwordLen = EEPROM.read(passwordLenAdr);

  if (ssidLen < 1 || ssidLen > 32 || passwordLen < 8 && passwordLen != 0  || passwordLen > 32) {
    reset();
    return;
  }

  ssid = "";
  password = "";
  for (int i = 0; i < ssidLen; i++) ssid += (char)EEPROM.read(ssidAdr + i);
  for (int i = 0; i < passwordLen; i++) password += (char)EEPROM.read(passwordAdr + i);

  ssidHidden = (bool)EEPROM.read(ssidHiddenAdr);

  if ((int)EEPROM.read(apChannelAdr) >= 1 && (int)EEPROM.read(apChannelAdr) <= 14) {
    apChannel = (int)EEPROM.read(apChannelAdr);
  } else {
    apChannel = 1;
  }
  for(int i=0; i<6; i++){
    macAP.setAt((uint8_t)EEPROM.read(macAPAdr+i),i);
  }
  if(!macAP.valid()) macAP.set(defaultMacAP);
  isMacAPRand = (bool)EEPROM.read(isMacAPRandAdr);

  apScanHidden = (bool)EEPROM.read(apScanHiddenAdr);

  deauthReason = EEPROM.read(deauthReasonAdr);
  attackTimeout = eepromReadInt(attackTimeoutAdr);
  attackPacketRate = EEPROM.read(attackPacketRateAdr);
  clientScanTime = EEPROM.read(clientScanTimeAdr);
  useLed = (bool)EEPROM.read(useLedAdr);
  channelHop = (bool)EEPROM.read(channelHopAdr);
  multiAPs = (bool)EEPROM.read(multiAPsAdr);
  multiAttacks = (bool)EEPROM.read(multiAttacksAdr);
  macInterval = eepromReadInt(macIntervalAdr);
  beaconInterval = (bool)EEPROM.read(beaconIntervalAdr);
  setLedPin((int)EEPROM.read(ledPinAdr));
  isSettingsLoaded = 1;
}

void Settings::reset() {
  if (debug) Serial.print("reset settings...");

  ssid = "pwned";
  password = "deauther"; //must have at least 8 characters
  ssidHidden = false;
  apChannel = 1;

  ssidLen = ssid.length();
  passwordLen = password.length();
  macAP = defaultMacAP;
  isMacAPRand = 0;

  apScanHidden = true;

  deauthReason = 0x01;
  attackTimeout = 5 * 60;
  attackPacketRate = 10;
  clientScanTime = 15;
  useLed = true;
  channelHop = false;
  multiAPs = false;
  multiAttacks = false;
  macInterval = 4;
  beaconInterval = false;
  ledPin = 2;

  if (debug) Serial.println("done");

  save();
}

void Settings::save() {
  ssidLen = ssid.length();
  passwordLen = password.length();

  EEPROM.write(ssidLenAdr, ssidLen);
  EEPROM.write(passwordLenAdr, passwordLen);
  for (int i = 0; i < ssidLen; i++) EEPROM.write(ssidAdr + i, ssid[i]);
  for (int i = 0; i < passwordLen; i++) EEPROM.write(passwordAdr + i, password[i]);

  EEPROM.write(ssidHiddenAdr, ssidHidden);
  EEPROM.write(apChannelAdr, apChannel);

  EEPROM.write(isMacAPRandAdr, isMacAPRand);

  for(int i=0; i<6; i++){
    EEPROM.write(macAPAdr+i, macAP._get(i));
  }

  EEPROM.write(apScanHiddenAdr, apScanHidden);

  EEPROM.write(deauthReasonAdr, deauthReason);

  eepromWriteInt(attackTimeoutAdr, attackTimeout);

  EEPROM.write(attackPacketRateAdr, attackPacketRate);
  EEPROM.write(clientScanTimeAdr, clientScanTime);
  EEPROM.write(useLedAdr, useLed);
  EEPROM.write(channelHopAdr, channelHop);
  EEPROM.write(multiAPsAdr, multiAPs);
  EEPROM.write(multiAttacksAdr, multiAttacks);
  EEPROM.write(checkNumAdr, checkNum);
  eepromWriteInt(macIntervalAdr, macInterval);
  EEPROM.write(beaconIntervalAdr, beaconInterval);
  EEPROM.write(ledPinAdr, ledPin);
  EEPROM.commit();

  if (debug) {
    info();
    Serial.println("settings saved");
  }
}

void Settings::info() {
  Serial.println("Settings:");
  Serial.println("SSID: " + ssid);
  Serial.println("SSID length: " + (String)ssidLen);
  Serial.println("SSID hidden: " + (String)ssidHidden);
  Serial.println("password: " + password);
  Serial.println("password length: " + (String)passwordLen);
  Serial.println("channel: " + (String)apChannel);
  Serial.println("Default MAC AP: " + defaultMacAP.toString());
  Serial.println("Saved MAC AP: " + macAP.toString());
  Serial.println("MAC AP random: " + (String)isMacAPRand);
  Serial.println("Scan hidden APs: " + (String)apScanHidden);
  Serial.println("deauth reson: " + (String)(int)deauthReason);
  Serial.println("attack timeout: " + (String)attackTimeout);
  Serial.println("attack packet rate: " + (String)attackPacketRate);
  Serial.println("client scan time: " + (String)clientScanTime);
  Serial.println("use built-in LED: " + (String)useLed);
  Serial.println("channel hopping: " + (String)channelHop);
  Serial.println("multiple APs: " + (String)multiAPs);
  Serial.println("multiple Attacks: " + (String)multiAttacks);
  Serial.println("mac change interval: " + (String)macInterval);
  Serial.println("1s beacon interval: " + (String)beaconInterval);
  Serial.println("LED Pin: " + (String)ledPin);
}

size_t Settings::getSize() {
  String json = "{";
  size_t jsonSize = 0;

  json += "\"ssid\":\"" + ssid + "\",";
  json += "\"ssidHidden\":" + (String)ssidHidden + ",";
  json += "\"password\":\"" + password + "\",";
  json += "\"apChannel\":" + (String)apChannel + ",";
  json += "\"macAp\":\"" + macAP.toString() + "\",";
  json += "\"randMacAp\":" + (String)isMacAPRand + ",";
  json += "\"apScanHidden\":" + (String)apScanHidden + ",";
  json += "\"deauthReason\":" + (String)(int)deauthReason + ",";
  json += "\"attackTimeout\":" + (String)attackTimeout + ",";
  json += "\"attackPacketRate\":" + (String)attackPacketRate + ",";
  json += "\"clientScanTime\":" + (String)clientScanTime + ",";
  json += "\"useLed\":" + (String)useLed + ",";
  json += "\"channelHop\":" + (String)channelHop + ",";
  json += "\"multiAPs\":" + (String)multiAPs + ",";
  json += "\"multiAttacks\":" + (String)multiAttacks + ",";
  json += "\"macInterval\":" + (String)macInterval + ",";
  json += "\"beaconInterval\":" + (String)beaconInterval + ",";
  json += "\"ledPin\":" + (String)ledPin + "}";
  jsonSize += json.length();

  return jsonSize;
}

void Settings::send() {
  if (debug) Serial.println("getting settings json");
  sendHeader(200, "text/json", getSize());

  String json = "{";
  json += "\"ssid\":\"" + ssid + "\",";
  json += "\"ssidHidden\":" + (String)ssidHidden + ",";
  json += "\"password\":\"" + password + "\",";
  json += "\"apChannel\":" + (String)apChannel + ",";
  json += "\"macAp\":\"" + macAP.toString() + "\",";
  json += "\"randMacAp\":" + (String)isMacAPRand + ",";
  json += "\"apScanHidden\":" + (String)apScanHidden + ",";
  json += "\"deauthReason\":" + (String)(int)deauthReason + ",";
  json += "\"attackTimeout\":" + (String)attackTimeout + ",";
  json += "\"attackPacketRate\":" + (String)attackPacketRate + ",";
  json += "\"clientScanTime\":" + (String)clientScanTime + ",";
  json += "\"useLed\":" + (String)useLed + ",";
  json += "\"channelHop\":" + (String)channelHop + ",";
  json += "\"multiAPs\":" + (String)multiAPs + ",";
  json += "\"multiAttacks\":" + (String)multiAttacks + ",";
  json += "\"macInterval\":" + (String)macInterval + ",";
  json += "\"beaconInterval\":" + (String)beaconInterval + ",";
  json += "\"ledPin\":" + (String)ledPin + "}";
  sendToBuffer(json);
  sendBuffer();

  if (debug) Serial.println("\ndone");

}
