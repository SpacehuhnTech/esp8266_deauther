#include "Settings.h"

Settings::Settings(){

}

void Settings::load(){ 
  ssidLen = EEPROM.read(ssidLenAdr);
  passwordLen = EEPROM.read(passwordLenAdr);
  
  if(ssidLen < 1 || ssidLen > 32 || passwordLen < 8 || passwordLen > 32) reset();
  for(int i=0;i<ssidLen;i++) ssid[i] = EEPROM.read(ssidAdr+i);
  for(int i=0;i<passwordLen;i++) password[i] = EEPROM.read(passwordAdr+i);
  
  deauthReason = EEPROM.read(deauthReasonAdr);
  attackTimeout = eepromReadInt(attackTimeoutAdr);
  attackPacketRate = EEPROM.read(attackPacketRateAdr);
  clientScanTime = EEPROM.read(clientScanTimeAdr);
}

void Settings::reset(){
  if(debug) Serial.print("reset settings...");
  
  ssid = "pwned";
  password = "deauther"; //must have at least 8 characters

  ssidLen = ssid.length();
  passwordLen = password.length();
    
  deauthReason = 0x01;
  attackTimeout = 5*60;
  attackPacketRate = 10;
  attackMacInterval = 4;
  
  clientScanTime = 15;
  
  if(debug) Serial.println("done");
  
  save();
}

void Settings::save(){
  if(debug) Serial.print("saving settings...");
  
  EEPROM.write(ssidLenAdr,ssidLen);
  EEPROM.write(passwordLenAdr,passwordLen);
  
  for(int i=0;i<ssidLen;i++) EEPROM.write(ssidAdr+i,ssid[i]);
  for(int i=0;i<passwordLen;i++) EEPROM.write(passwordAdr+i,password[i]);
  
  EEPROM.write(deauthReasonAdr, deauthReason);
  EEPROM.write(deauthReasonAdr, deauthReason);

  eepromWriteInt(attackTimeoutAdr, attackTimeout);

  EEPROM.write(attackPacketRateAdr, attackPacketRate);
  EEPROM.write(attackMacIntervalAdr, attackMacInterval);
  EEPROM.write(clientScanTimeAdr, clientScanTime);

  if(debug){
    info();
    Serial.println("done");
  }
  
}

void Settings::info(){
  Serial.println("settings:");
  Serial.println("SSID: "+ssid);
  Serial.println("SSID length: "+(String)ssidLen);
  Serial.println("password: "+password);
  Serial.println("password length: "+(String)passwordLen);
  Serial.println("deauth reson: "+(String)(int)deauthReason);
  Serial.println("attack timeout: "+(String)attackTimeout);
  Serial.println("attack packet rate: "+(String)attackPacketRate);
  Serial.println("client scan time: "+(String)clientScanTime);
}

