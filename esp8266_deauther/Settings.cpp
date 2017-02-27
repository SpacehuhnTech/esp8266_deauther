#include "Settings.h"

Settings::Settings(){

}

void Settings::load(){ 
  ssidLen = EEPROM.read(ssidLenAdr);
  passwordLen = EEPROM.read(passwordLenAdr);
  
  if(ssidLen < 1 || ssidLen > 32 || passwordLen < 8 || passwordLen > 32) reset();
  else{
    ssid = "";
    password = "";
    for(int i=0;i<ssidLen;i++) ssid += (char)EEPROM.read(ssidAdr+i);
    for(int i=0;i<passwordLen;i++) password += (char)EEPROM.read(passwordAdr+i);
    
    deauthReason = EEPROM.read(deauthReasonAdr);
    attackTimeout = eepromReadInt(attackTimeoutAdr);
    attackPacketRate = EEPROM.read(attackPacketRateAdr);
    clientScanTime = EEPROM.read(clientScanTimeAdr);
    attackEncrypted = (bool)EEPROM.read(attackEncryptedAdr);
  }
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
  clientScanTime = 15;
  attackEncrypted = false;
  
  if(debug) Serial.println("done");
  
  save();
}

void Settings::save(){
  ssidLen = ssid.length();
  passwordLen = password.length();
  
  EEPROM.write(ssidLenAdr,ssidLen);
  EEPROM.write(passwordLenAdr,passwordLen);
  
  for(int i=0;i<ssidLen;i++) EEPROM.write(ssidAdr+i,ssid[i]);
  for(int i=0;i<passwordLen;i++) EEPROM.write(passwordAdr+i,password[i]);
  
  EEPROM.write(deauthReasonAdr, deauthReason);

  eepromWriteInt(attackTimeoutAdr, attackTimeout);

  EEPROM.write(attackPacketRateAdr, attackPacketRate);
  EEPROM.write(clientScanTimeAdr, clientScanTime);
  EEPROM.write(attackEncryptedAdr, attackEncrypted);
  EEPROM.commit();
  
  if(debug){
    info();
    Serial.println("settings saved");
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
  Serial.println("attack SSID encrypted: "+(String)attackEncrypted);
}

String Settings::get(){
  String json = "{";
  
  json += "\"ssid\":\""+ssid+"\",";
  json += "\"password\":\""+password+"\",";
  json += "\"deauthReason\":"+(String)(int)deauthReason+",";
  json += "\"attackTimeout\":"+(String)attackTimeout+",";
  json += "\"attackPacketRate\":"+(String)attackPacketRate+",";
  json += "\"clientScanTime\":"+(String)clientScanTime+",";
  json += "\"attackEncrypted\":"+(String)attackEncrypted+",";

  json += "\"nameList\":[";
  for(int i=0;i<nameList.len;i++){
    json += "{";
    json += "\"n\":\""+nameList.getName(i)+"\",";
    json += "\"m\":\""+nameList.getMac(i).toString()+"\",";
    json += "\"v\":\""+data_getVendor(nameList.getMac(i)._get(0), nameList.getMac(i)._get(1), nameList.getMac(i)._get(2))+"\"";
    json += "}";
    if(i!=nameList.len-1) json += ",";
  }

  json += "] }";
  return json;
}
