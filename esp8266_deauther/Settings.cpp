#include "Settings.h"

Settings::Settings(){

}

void Settings::load(){ 
  ssidLen = EEPROM.read(ssidLenAdr);
  passwordLen = EEPROM.read(passwordLenAdr);
  
  if(ssidLen < 1 || ssidLen > 32 || passwordLen < 8 && passwordLen != 0  || passwordLen > 32) reset();
  else{
    ssid = "";
    password = "";
    for(int i=0;i<ssidLen;i++) ssid += (char)EEPROM.read(ssidAdr+i);
    for(int i=0;i<passwordLen;i++) password += (char)EEPROM.read(passwordAdr+i);
    
    ssidHidden = (bool)EEPROM.read(ssidHiddenAdr);

    if((int)EEPROM.read(apChannelAdr) >= 1 && (int)EEPROM.read(apChannelAdr) <= 14){
      apChannel = (int)EEPROM.read(apChannelAdr);
    } else {
      apChannel = 1;
    }
    
    apScanHidden = (bool)EEPROM.read(apScanHiddenAdr);
    deauthReason = EEPROM.read(deauthReasonAdr);
    attackTimeout = eepromReadInt(attackTimeoutAdr);
    attackPacketRate = EEPROM.read(attackPacketRateAdr);
    clientScanTime = EEPROM.read(clientScanTimeAdr);
    attackEncrypted = (bool)EEPROM.read(attackEncryptedAdr);
    useLed = (bool)EEPROM.read(useLedAdr);
    channelHop = (bool)EEPROM.read(channelHopAdr);
  }
}

void Settings::reset(){
  if(debug) Serial.print("Resetting settings..");
  
  ssid = "WiFi";
  password = "deauther"; //must have at least 8 characters
  ssidHidden = false;
  apChannel = 1;
  ssidLen = ssid.length();
  passwordLen = password.length();
  apScanHidden = true;
  deauthReason = 0x01;
  attackTimeout = 0;
  attackPacketRate = 10;
  clientScanTime = 15;
  attackEncrypted = true;
  useLed = true;
  channelHop = false;
  
  if(debug) Serial.println("Done!");
  
  save();
}

void Settings::save(){
  ssidLen = ssid.length();
  passwordLen = password.length();
  EEPROM.write(ssidLenAdr, ssidLen);
  EEPROM.write(passwordLenAdr, passwordLen);
  for(int i=0;i<ssidLen;i++) EEPROM.write(ssidAdr+i,ssid[i]);
  for(int i=0;i<passwordLen;i++) EEPROM.write(passwordAdr+i,password[i]);
  EEPROM.write(ssidHiddenAdr, ssidHidden);
  EEPROM.write(apChannelAdr, apChannel);
  EEPROM.write(apScanHiddenAdr, apScanHidden);
  EEPROM.write(deauthReasonAdr, deauthReason);
  eepromWriteInt(attackTimeoutAdr, attackTimeout);
  EEPROM.write(attackPacketRateAdr, attackPacketRate);
  EEPROM.write(clientScanTimeAdr, clientScanTime);
  EEPROM.write(attackEncryptedAdr, attackEncrypted);
  EEPROM.write(useLedAdr, useLed);
  EEPROM.write(channelHopAdr, channelHop);
  EEPROM.commit();
  
  if(debug){
    info();
    Serial.println("Settings saved!");
  }
}

void Settings::info(){
  Serial.println("Settings:");
  Serial.println("SSID: "+ssid);
  Serial.println("SSID length: "+(String)ssidLen);
  Serial.println("SSID hidden: "+(String)ssidHidden);
  Serial.println("Password: "+password);
  Serial.println("Password length: "+(String)passwordLen);
  Serial.println("Channel: "+(String)apChannel);
  Serial.println("Scan hidden APs: "+(String)apScanHidden);
  Serial.println("Deauth reson: "+(String)(int)deauthReason);
  Serial.println("Attack timeout: "+(String)attackTimeout);
  Serial.println("Attack packet rate: "+(String)attackPacketRate);
  Serial.println("Client scan time: "+(String)clientScanTime);
  Serial.println("Attack SSID encrypted: "+(String)attackEncrypted);
  Serial.println("Use built-in LED: "+(String)useLed);
  Serial.println("Channel hopping: "+(String)channelHop);
}

String Settings::get(){
  if(debug) Serial.println("Getting settings json..");
  String json = "{";
  
  json += "\"ssid\":\""+ssid+"\",";
  json += "\"ssidHidden\":"+(String)ssidHidden+",";
  json += "\"password\":\""+password+"\",";
  json += "\"apChannel\":"+(String)apChannel+",";
  json += "\"apScanHidden\":"+(String)apScanHidden+",";
  json += "\"deauthReason\":"+(String)(int)deauthReason+",";
  json += "\"attackTimeout\":"+(String)attackTimeout+",";
  json += "\"attackPacketRate\":"+(String)attackPacketRate+",";
  json += "\"clientScanTime\":"+(String)clientScanTime+",";
  json += "\"attackEncrypted\":"+(String)attackEncrypted+",";
  json += "\"useLed\":"+(String)useLed+",";
  json += "\"channelHop\":"+(String)channelHop+",";

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
  if(debug){
    Serial.println(json);
    Serial.println("Done!");
  }
  return json;
}