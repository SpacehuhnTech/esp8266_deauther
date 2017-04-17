#include "APScan.h"

APScan::APScan(){
  
}

bool APScan::start(){
    if(debug){
      Serial.println("Starting AP scan..");
      Serial.println("MAC - Ch - RSSI - Encrypt. - SSID - Hidden");
    }
    aps._clear();
    for(int i=0;i<maxAPScanResults;i++) selected[i] = true; //Auto select APs
    results = WiFi.scanNetworks(false, settings.apScanHidden); // lets scanNetworks return hidden APs. (async = false & show_hidden = true)
    
    for(int i=0;i<results && i<maxAPScanResults;i++){
      Mac _ap;
      _ap.set(WiFi.BSSID(i)[0],WiFi.BSSID(i)[1],WiFi.BSSID(i)[2],WiFi.BSSID(i)[3],WiFi.BSSID(i)[4],WiFi.BSSID(i)[5]);
      aps.add(_ap);
      channels[i] = WiFi.channel(i);
      rssi[i] = WiFi.RSSI(i);
      encryption[i] = WiFi.encryptionType(i);
      hidden[i] = WiFi.isHidden(i);
      String _ssid = WiFi.SSID(i);
      _ssid.replace("\"","\\\"");
      _ssid.toCharArray(names[i],33);
      if(debug){
        Serial.print((String)i);
        Serial.print(" - ");
        _ap._print();
        Serial.print(" - ");
        Serial.print(channels[i]);
        Serial.print(" - ");
        Serial.print(rssi[i]);
        Serial.print(" - ");
        Serial.print(getEncryption(encryption[i]));
        Serial.print(" - ");
        Serial.print(names[i]);
        Serial.print(" - ");
        Serial.print(hidden[i]);
        Serial.println();
      }
    }
    
    if(debug) Serial.println("Scan done!");
    return true;
}

String APScan::getEncryption(int code){
  switch (code) {
    case ENC_TYPE_NONE:
      return "none";
      break;
    case ENC_TYPE_WEP:
      return "WEP";
      break;
    case ENC_TYPE_TKIP:
      return "WPA";
      break;
    case ENC_TYPE_CCMP:
      return "WPA2";
      break;
    case ENC_TYPE_AUTO:
      return "WPA*";
      break;
  }
  return "?";
}

String APScan::getAPName(int num){ 
    if(isHidden(num)) return "* Hidden SSID *";
    return names[num]; 
}
String APScan::getAPEncryption(int num){ return getEncryption(encryption[num]); }
String APScan::getAPMac(int num){ return aps._get(num).toString(); }
bool APScan::getAPSelected(int num){ return selected[num]; }
bool APScan::isHidden(int num){ return hidden[num]; }
int APScan::getAPRSSI(int num){ return rssi[num]; }
int APScan::getAPChannel(int num){ return channels[num]; }

int APScan::getFirstTarget(){
  for(int i=0;i<maxAPScanResults;i++){
    if(isSelected(i)) return i;
  }
  return -1;
}

void APScan::sendResults(){
  if(debug) Serial.print("Sending AP scan result JSON..");

  size_t _size = 10;
  for(int i=0;i<results && i<maxAPScanResults;i++){
      _size += 61;
      _size += String(i).length();
      _size += String(getAPChannel(i)).length();
      _size += getAPName(i).length();
      _size += String(getAPRSSI(i)).length();
      
      if((i!=results-1) && (i!=maxAPScanResults-1)) _size++;
  }

  sendHeader(200, "text/json", _size);
  
  String json;
  int bufc = 0; //bufferCounter
  json = "{\"aps\":[";

  sendToBuffer(json);
  
  for(int i=0;i<results && i<maxAPScanResults;i++){
    if(debug) Serial.print(".");
    json = "{";
    json += "\"i\":"+(String)i+",";
    json += "\"c\":"+(String)getAPChannel(i)+",";
    json += "\"m\":\""+getAPMac(i)+"\",";
    json += "\"ss\":\""+getAPName(i)+"\",";
    json += "\"r\":"+(String)getAPRSSI(i)+",";
    json += "\"e\":"+(String)encryption[i]+",";
    json += "\"se\":"+(String)getAPSelected(i);
    json += "}";
    if((i!=results-1) && (i!=maxAPScanResults-1)) json += ",";
    
    sendToBuffer(json);
  
  }
  json = "]}";
  sendToBuffer(json);
  sendBuffer();
    
  if(debug) Serial.println("Done!");

}

String APScan::getResults(){
  if(debug) Serial.print("Getting AP scan result JSON..");
  String json = "{ \"aps\":[ ";
  for(int i=0;i<results && i<maxAPScanResults;i++){
    if(debug) Serial.print(".");
    json += "{";
    json += "\"i\":"+(String)i+",";
    json += "\"c\":"+(String)getAPChannel(i)+",";
    json += "\"m\":\""+getAPMac(i)+"\",";
    json += "\"ss\":\""+getAPName(i)+"\",";
    json += "\"r\":"+(String)getAPRSSI(i)+",";
    json += "\"e\":"+(String)encryption[i]+",";
    json += "\"se\":"+(String)getAPSelected(i);
    json += "}";
    if((i!=results-1) && (i!=maxAPScanResults-1)) json += ",";
  }
  json += "] }";
  if(debug){
    Serial.println(json);
    Serial.println("Done!");
  }
  return json;
}

String APScan::getResult(int i){
  if(debug) Serial.print("Getting AP scan result JSON for ID " + String(i));
  String json = "{ \"aps\":[ ";
  if(debug) Serial.print(".");
  json += "{";
  json += "\"i\":"+(String)i+",";
  json += "\"c\":"+(String)getAPChannel(i)+",";
  json += "\"m\":\""+getAPMac(i)+"\",";
  json += "\"ss\":\""+getAPName(i)+"\",";
  json += "\"r\":"+(String)getAPRSSI(i)+",";
  json += "\"e\":"+(String)encryption[i]+",";
  json += "\"se\":"+(String)getAPSelected(i);
  json += "}";
  json += "] }";

  if(debug){
    Serial.println(json);
    Serial.println("Done!");
  }
  return json;
}

void APScan::select(int num){
  if(debug) Serial.println("Select "+(String)num+" - "+!selected[num]);
  if(selected[num]){
    selected[num] = false;
    selectedSum--;
  }else{
    selected[num] = true;
    selectedSum++;
  }
}

bool APScan::isSelected(int num){
  return selected[num];
}