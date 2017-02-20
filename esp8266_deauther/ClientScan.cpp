#include "ClientScan.h"

ClientScan::ClientScan(){
  broadcast.set(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
  zero.set(0x00,0x00,0x00,0x00,0x00,0x00);
}

void ClientScan::start(int _time){
  clients._clear();
  for(int i=0;i<maxResults;i++) selected[i] = false;
  results = 0;
  timeout = _time;
  target.setMac(apScan.getTarget());
  sniffing = true;

  startTime = millis();

  /*Serial.print("starting scan on: ");
  target._println();*/

  wifi_promiscuous_enable(0);
  WiFi.disconnect();
  wifi_set_opmode(STATION_MODE);
  wifi_set_channel(apScan.getAPChannel(apScan.selected));
  wifi_promiscuous_enable(1);
}

bool ClientScan::stop(){
  long curTime = millis();
  if(curTime - startTime >= timeout*1000){
    sniffing = false;
    wifi_promiscuous_enable(0);
    
    /*for(int i=0;i<results && i<maxResults;i++){
      Serial.print(i);
      Serial.print(": ");
      Serial.print(getClientPackets(i));
      Serial.print(" ");
      Serial.print(getClientVendor(i));
      Serial.print(" ");
      Serial.print(getClientMac(i).toString());
      Serial.print(" ");
      Serial.print(getClientSelected(i));
      Serial.println("");
    }*/
  
    return true;
  }
  else return false;

  
}

void ClientScan::packetSniffer(uint8_t *buf, uint16_t len){
  if(sniffing && len>15){
    from.set(buf[16],buf[17],buf[18],buf[19],buf[20],buf[21]);
    to.set(buf[22],buf[23],buf[24],buf[25],buf[26],buf[27]);

    if(target.compare(from)){
      if(buf[22] == 0xFF && buf[23] == 0xFF && buf[24] == 0xFF && buf[25] == 0xFF && buf[26] == 0xFF && buf[27] == 0xFF){
        Serial.print(len);
        Serial.print(" : ");
        PrintHex8(buf, len);
        Serial.println("");
      }
      int clientNum = clients.getNum(to);
      if(clientNum == -1 && results < maxResults){
        data_getVendor(to._get(0),to._get(1),to._get(2)).toCharArray(vendors[results],9);
        results++;
        packets[clients.add(to)]++;
      }else packets[clientNum]++;
      
      /*
      Serial.println("found:");
      from._print();
      Serial.print(" => ");
      to._print();
      Serial.println("");*/
    }
  }
}

String ClientScan::getClientName(int num){ return nameList.get(clients._get(num)); }
int ClientScan::getClientPackets(int num){ return packets[clients.getNum(clients._get(num))]; }
String ClientScan::getClientVendor(int num){ return vendors[num]; }
Mac ClientScan::getClientMac(int num){ return clients._get(num); }
bool ClientScan::getClientSelected(int num){ return selected[num]; }

String ClientScan::getResults(){
  
  String json = "{ \"clients\":[";
  for(int i=0;i<results && i<maxResults;i++){
    json += "{";
    json += "\"id\": "+(String)i+",";
    json += "\"packets\": "+(String)getClientPackets(i)+",";
    json += "\"mac\": \""+getClientMac(i).toString()+"\",";
    json += "\"name\": \""+(String)nameList.get(getClientMac(i))+"\",";
    json += "\"vendor\": \""+(String)getClientVendor(i)+"\",";
    json += "\"selected\": "+(String)getClientSelected(i);
    json += "}";
    if((i!=results-1) && (i!=maxResults-1)) json += ",";
  }
  json += "] }";
  return json;
  
}

void ClientScan::select(int num){
  selected[num] = !selected[num];
}
