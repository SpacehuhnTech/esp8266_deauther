#include "APScan.h"

APScan::APScan(){
  
}

bool APScan::start(){
    if(debug){
      Serial.println("starting AP scan...");
      Serial.println("MAC - Ch - RSSI - Encrypt. - SSID - Vendor");
    }
    aps._clear();
    for(int i=0;i<maxResults;i++) selected[i] = false;
    results = WiFi.scanNetworks();
    
    for(int i=0;i<results && i<maxResults;i++){
      Mac _ap;
      _ap.set(WiFi.BSSID(i)[0],WiFi.BSSID(i)[1],WiFi.BSSID(i)[2],WiFi.BSSID(i)[3],WiFi.BSSID(i)[4],WiFi.BSSID(i)[5]);
      aps.add(_ap);
      channels[i] = WiFi.channel(i);
      rssi[i] = WiFi.RSSI(i);
      getEncryption(WiFi.encryptionType(i)).toCharArray(encryption[i],5);
      String _ssid;
      _ssid = WiFi.SSID(i);
      _ssid.replace("\"","\\\"");
      _ssid.toCharArray(names[i],33);
      data_getVendor(WiFi.BSSID(i)[0],WiFi.BSSID(i)[1],WiFi.BSSID(i)[2]).toCharArray(vendors[i],9);
      if(debug){
        _ap._print();
        Serial.print(" - ");
        Serial.print(channels[i]);
        Serial.print(" - ");
        Serial.print(rssi[i]);
        Serial.print(" - ");
        Serial.print(encryption[i]);
        Serial.print(" - ");
        Serial.print(names[i]);
        Serial.print(" - ");
        Serial.print(vendors[i]);
        Serial.println();
      }
    }
    
    if(debug) Serial.println("scan done");
    if(debug) Serial.println(getResults());
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
    default:
      return "?";
  }
}

String APScan::getAPName(int num){ return names[num]; }
String APScan::getAPEncryption(int num){ return encryption[num]; }
String APScan::getAPVendor(int num){ return vendors[num]; }
String APScan::getAPMac(int num){ return aps._get(num).toString(); }
String APScan::getAPSelected(int num){
  if(selected[num]) return "true";
  else return "false";  
}
int APScan::getAPRSSI(int num){ return rssi[num]; }
int APScan::getAPChannel(int num){ return channels[num]; }

int APScan::getFirstTarget(){
  for(int i=0;i<maxResults;i++){
    if(isSelected(i)) return i;
  }
  return -1;
}

String APScan::getResults(){
  if(debug) Serial.print("getting AP scan result JSON ");
  String json = "{ \"aps\":[ ";
  for(int i=0;i<results && i<maxResults;i++){
    if(debug) Serial.print(".");
    json += "{";
    json += "\"id\": "+(String)i+",";
    json += "\"channel\": "+(String)getAPChannel(i)+",";
    json += "\"mac\": \""+getAPMac(i)+"\",";
    json += "\"ssid\": \""+getAPName(i)+"\",";
    json += "\"rssi\": "+(String)getAPRSSI(i)+",";
    json += "\"encryption\": \""+getAPEncryption(i)+"\",";
    json += "\"vendor\": \""+getAPVendor(i)+"\",";
    json += "\"selected\": "+getAPSelected(i);
    json += "}";
    if((i!=results-1) && (i!=maxResults-1)) json += ",";
  }
  json += "] }";
  if(debug) Serial.println("done");
  return json;
}

void APScan::select(int num){
  if(debug) Serial.println("seect "+(String)num+" - "+!selected[num]);
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
