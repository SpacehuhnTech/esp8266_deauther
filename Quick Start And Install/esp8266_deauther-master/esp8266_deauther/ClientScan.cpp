#include "ClientScan.h"

ClientScan::ClientScan() {
  broadcast.set(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
  zero.set(0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}

void ClientScan::clearList() {
  clients._clear();
}

void ClientScan::start(int _time) {
  Serial.println();
  Serial.println("starting client scan");

  clearList();
  for (int i = 0; i < maxClientScanResults; i++) {
    selected[i] = false;
    packets[i] = 0;
  }

  for (int i = 0; i < 13; i++) channels[i] = 0;

  results = 0;
  timeout = _time;
  sniffing = true;

  channelsNum = 0;
  curChannel = 0;

  for (int i = 0; i < apScan.results; i++) {
    if (apScan.isSelected(i)) {
      if (!intInArray(apScan.getAPChannel(i), channels)) {
        channels[channelsNum] = apScan.getAPChannel(i);
        channelsNum++;
      }
    }
  }

  wifi_promiscuous_enable(0);
  WiFi.disconnect();
  wifi_set_opmode(STATION_MODE);
  wifi_set_channel(channels[curChannel]);
  wifi_promiscuous_enable(1);

  if (debug) Serial.println("set channel to " + (String)channels[curChannel]);
  curChannel++;

  startTime = millis();
  sniffing = true;
}

bool ClientScan::stop() {
  long curTime = millis();
  if (curTime - startTime >= (timeout * 1000) / channelsNum && curChannel < channelsNum) {
    if (debug) Serial.println("changing to channel " + (String)channels[curChannel]);
    wifi_set_channel(channels[curChannel]);
    curChannel++;
  }
  else if (curTime - startTime >= timeout * 1000) {
    sniffing = false;
    wifi_promiscuous_enable(0);

    Serial.println("stopping client scan after " + (String)(curTime - startTime) + "ms");
    if (debug) {
      for (int i = 0; i < results && i < maxClientScanResults; i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.print(getClientPackets(i));
        Serial.print(" ");
        Serial.print(getClientVendor(i));
        Serial.print(" ");
        Serial.print(getClientMac(i).toString());
        /*Serial.print(" ");
          Serial.print(getClientSelected(i));*/
        Serial.println();
      }
    }
    return true;
  }
  return false;
}

int ClientScan::add(Mac adr){
  int clientNum = clients.getNum(adr);
  if (clientNum == -1 && results < maxClientScanResults) {
    data_getVendor(adr._get(0), adr._get(1), adr._get(2)).toCharArray(vendors[results], 9);
    connectedToAp[results] = -1;
    results++;
    int clientNum = clients.add(adr);
  }
  return clientNum;
}

void ClientScan::packetSniffer(uint8_t *buf, uint16_t len) {
  if (sniffing && len > 27) {
    from.set(buf[16], buf[17], buf[18], buf[19], buf[20], buf[21]);
    to.set(buf[22], buf[23], buf[24], buf[25], buf[26], buf[27]);

    for (int i = 0; i < apScan.results; i++) {
      if (apScan.isSelected(i)) {
        if (apScan.aps._get(i).compare(from) && !broadcast.compare(to)) {
          int clientNum = add(to);
          connectedToAp[clientNum] = i;
          packets[clientNum]++;
          if (clientNum == -1 && results < maxClientScanResults) {
            if (debug) {
              Serial.print("found: ");
              from._print();
              Serial.print(" => ");
              to._print();
              Serial.println("");
            }
          }
        }
      }
    }
  }
}

String ClientScan::getClientName(int num) {
  return nameList.getByMac(getClientMac(num));
}
int ClientScan::getClientPackets(int num) {
  return packets[clients.getNum(clients._get(num))];
}
String ClientScan::getClientVendor(int num) {
  return vendors[num];
}
Mac ClientScan::getClientMac(int num) {
  return clients._get(num);
}
int ClientScan::getClientConnectedAp(int num) {
  return connectedToAp[num];
}
bool ClientScan::getClientSelected(int num) {
  return selected[num];
}
int ClientScan::getFirstClient() {
  for (int i = 0; i < maxClientScanResults; i++) {
    if (getClientSelected(i)) return i;
  }
  return -1;
}

void ClientScan::select(int num) {
  if(num < 0){
    if(num == -1){
      for(int i=0; i<results; i++){selected[i] = true;}
      selectedResults = results;
    } else {
      for(int i=0; i<results; i++){selected[i] = false;}
      selectedResults = 0;
    }  
  } else {
    selected[num] = !selected[num];
    if (selected[num]) selectedResults++;
    else selectedResults--;
  }
}

size_t ClientScan::getSize(){
  size_t jsonSize = 0;
  String json = "{\"clients\":[";
  jsonSize += json.length();
  for (int i = 0; i < results && i < maxClientScanResults; i++) {
      json = "{";
      json += "\"i\":" + (String)i + ",";
      json += "\"p\":" + (String)getClientPackets(i) + ",";
      json += "\"m\":\"" + getClientMac(i).toString() + "\",";
      json += "\"n\":\"" + (String)nameList.getByMac(getClientMac(i)) + "\",";
      json += "\"l\":" + (String)nameList.getNumByMac(getClientMac(i)) + ",";
      json += "\"v\":\"" + (String)getClientVendor(i) + "\",";
      json += "\"s\":" + (String)getClientSelected(i) + ",";
      if(getClientConnectedAp(i)>=0) json += "\"a\":\"" + (String)apScan.getAPName(getClientConnectedAp(i)) + "\"";
      else json += "\"a\":\"?\"";
      json += "}";
      if ((i != results - 1) && (i != maxClientScanResults - 1)) json += ",";
      jsonSize += json.length();
  }
  json = "],\"nameList\":[";
  jsonSize += json.length();
  
  for (int i = 0; i < nameList.len; i++) {
    json = "{";
    json += "\"n\":\"" + nameList.get(i) + "\",";
    json += "\"m\":\"" + nameList.getMac(i).toString() + "\"";
    //json += "\"v\":\"" + data_getVendor(nameList.getMac(i)._get(0), nameList.getMac(i)._get(1), nameList.getMac(i)._get(2)) + "\"";
    json += "}";
    if (i != nameList.len - 1) json += ",";
    jsonSize += json.length();
  }
  jsonSize += 2;
    
  return jsonSize;
}

void ClientScan::send() {
  size_t _size = getSize();
  sendHeader(200, "text/json", _size);
  if (debug) Serial.println("getting settings json ("+(String)_size+")");

  String json;
  sendToBuffer("{\"clients\":[");
  for (int i = 0; i < results && i < maxClientScanResults; i++) {
      json = "{";
      json += "\"i\":" + (String)i + ",";
      json += "\"p\":" + (String)getClientPackets(i) + ",";
      json += "\"m\":\"" + getClientMac(i).toString() + "\",";
      json += "\"n\":\"" + (String)nameList.getByMac(getClientMac(i)) + "\",";
      json += "\"l\":" + (String)nameList.getNumByMac(getClientMac(i)) + ",";
      json += "\"v\":\"" + (String)getClientVendor(i) + "\",";
      json += "\"s\":" + (String)getClientSelected(i) + ",";
      if(getClientConnectedAp(i)>=0) json += "\"a\":\"" + (String)apScan.getAPName(getClientConnectedAp(i)) + "\"";
      else json += "\"a\":\"?\"";
      json += "}";
      if ((i != results - 1) && (i != maxClientScanResults - 1)) json += ",";
      sendToBuffer(json);
  }
  sendToBuffer("],\"nameList\":[");
  
  for (int i = 0; i < nameList.len; i++) {
    json = "{";
    json += "\"n\":\"" + nameList.get(i) + "\",";
    json += "\"m\":\"" + nameList.getMac(i).toString() + "\"";
    //json += "\"v\":\"" + data_getVendor(nameList.getMac(i)._get(0), nameList.getMac(i)._get(1), nameList.getMac(i)._get(2)) + "\"";
    json += "}";
    if (i != nameList.len - 1) json += ",";
    sendToBuffer(json);
  }
  sendToBuffer("]}");

  sendBuffer();

  if(debug) Serial.println("\ndone");

}
