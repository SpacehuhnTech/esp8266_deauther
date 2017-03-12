#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

extern "C" {
#include "user_interface.h"
}

#include <EEPROM.h>
#include "data.h"
#include "NameList.h"
#include "APScan.h"
#include "ClientScan.h"
#include "Attack.h"
#include "Settings.h"
#include "SSIDList.h"

/* ========== DEBUG ========== */
const bool debug = true;
/* ========== DEBUG ========== */

ESP8266WebServer server(80);

NameList nameList;

APScan apScan;
ClientScan clientScan;
Attack attack;
Settings settings;
SSIDList ssidList;

void sniffer(uint8_t *buf, uint16_t len) {
  clientScan.packetSniffer(buf, len);
}

void startWifi() {
  Serial.println("starting WiFi AP");
  WiFi.mode(WIFI_STA);
  wifi_set_promiscuous_rx_cb(sniffer);
  WiFi.softAP((const char*)settings.ssid.c_str(), (const char*)settings.password.c_str(), settings.apChannel, settings.ssidHidden); //for an open network without a password change to:  WiFi.softAP(ssid);
  Serial.println("SSID: " + settings.ssid);
  Serial.println("Password: " + settings.password);
  if (settings.password.length() < 8) Serial.println("WARNING: password must have at least 8 characters!");
  if (settings.ssid.length() < 1 || settings.ssid.length() > 32) Serial.println("WARNING: SSID length must be between 1 and 32 characters!");
}

void setup() {

  Serial.begin(115200);
  delay(2000);

  pinMode(2, OUTPUT);
  delay(50);
  digitalWrite(2, HIGH);
  
  EEPROM.begin(4096);
  
  settings.load();
  if (debug) settings.info();
  nameList.load();
  ssidList.load();

  Serial.println("");
  Serial.println("starting...");

  startWifi();
  attack.stopAll();
  attack.generate();

  /* ========== Web Server ========== */

  /* HTML sites */
  server.onNotFound(load404);

  server.on("/", loadIndex);
  server.on("/index.html", loadIndex);
  server.on("/clients.html", loadClients);
  server.on("/attack.html", loadAttack);
  server.on("/settings.html", loadSettings);
  server.on("/functions.js", loadFunctionsJS);

  /* header links */
  server.on ("/style.css", loadStyle);
  server.on ("/manifest.json", loadManifest);

  /* JSON */
  server.on("/APScanResults.json", sendAPResults);
  server.on("/APScan.json", startAPScan);
  server.on("/APSelect.json", selectAP);
  server.on("/ClientScan.json", startClientScan);
  server.on("/ClientScanResults.json", sendClientResults);
  server.on("/ClientScanTime.json", sendClientScanTime);
  server.on("/clientSelect.json", selectClient);
  server.on("/setName.json", setClientName);
  server.on("/attackInfo.json", sendAttackInfo);
  server.on("/attackStart.json", startAttack);
  server.on("/settings.json", getSettings);
  server.on("/settingsSave.json", saveSettings);
  server.on("/settingsReset.json", resetSettings);
  server.on("/deleteName.json", deleteName);
  server.on("/clearNameList.json", clearNameList);
  server.on("/editNameList.json", editClientName);
  server.on("/addSSID.json", addSSID);
  server.on("/cloneSSID.json", cloneSSID);
  server.on("/deleteSSID.json", deleteSSID);
  server.on("/randomSSID.json", randomSSID);
  server.on("/clearSSID.json", clearSSID);
  server.on("/resetSSID.json", resetSSID);
  server.on("/saveSSID.json", saveSSID);
  server.on("/restartESP.json", restartESP);

  server.begin();
}

void loop() {
  if (clientScan.sniffing) {
    if (clientScan.stop()) startWifi();
  } else {
    server.handleClient();
    attack.run();
  }
}

void load404() {
  server.send ( 200, "text/html", data_get404());
}
void loadIndex() {
  server.send ( 200, "text/html", data_getIndexHTML() );
}
void loadClients() {
  server.send ( 200, "text/html", data_getClientsHTML());
}
void loadAttack() {
  server.send ( 200, "text/html", data_getAttackHTML() );
}
void loadFunctionsJS() {
  server.send( 200, "text/javascript", data_getFunctionsJS() );
}
void loadStyle() {
  server.send ( 200, "text/css", data_getStyle() );
}
void loadManifest() {
  server.send ( 200, "text/css", data_getManifest() );
}
void loadSettings() {
  server.send( 200, "text/html", data_getSettingsHTML() );
}


//==========AP-Scan==========
void startAPScan() {
  if (apScan.start()) {
    server.send ( 200, "text/json", "true");
    attack.stopAll();
  }
}

void sendAPResults() {
  if (server.hasArg("apid")) {
    int apid = server.arg("apid").toInt();
    server.send ( 200, "text/json", apScan.getResult(apid));
  } else {
    server.send ( 200, "text/json", apScan.getResults());
  }
}

void selectAP() {
  if (server.hasArg("num")) {
    apScan.select(server.arg("num").toInt());
    server.send( 200, "text/json", "true");
    attack.stopAll();
  }
}

//==========Client-Scan==========
void startClientScan() {
  if (server.hasArg("time") && apScan.getFirstTarget() > -1 && !clientScan.sniffing) {
    server.send(200, "text/json", "true");
    clientScan.start(server.arg("time").toInt());
    attack.stopAll();
  } else server.send( 200, "text/json", "Error: no selected access point");
}

void sendClientResults() {
  server.send( 200, "text/json", clientScan.getResults() );
}
void sendClientScanTime() {
  server.send( 200, "text/json", (String)settings.clientScanTime );
}

void selectClient() {
  if (server.hasArg("num")) {
    clientScan.select(server.arg("num").toInt());
    attack.stop(0);
    server.send( 200, "text/json", "true");
  }
}

void setClientName() {
  if (server.hasArg("id") && server.hasArg("name")) {
    nameList.add(clientScan.getClientMac(server.arg("id").toInt()), server.arg("name"));
    server.send( 200, "text/json", "true");
  }
}

//==========Attack==========
void sendAttackInfo() {
  server.send( 200, "text/json", attack.getResults());
}

void startAttack() {
  if (server.hasArg("num")) {
    int _attackNum = server.arg("num").toInt();
    if (apScan.getFirstTarget() > -1 || _attackNum == 2 || _attackNum == 3) {
      attack.start(server.arg("num").toInt());
      server.send ( 200, "text/json", "true");
    } else server.send( 200, "text/json", "false");
  }
}

void addSSID() {
  ssidList.add(server.arg("name"));
  server.send( 200, "text/json", "true");
}

void cloneSSID() {
  ssidList.addClone(server.arg("name"));
  server.send( 200, "text/json", "true");
}

void deleteSSID() {
  ssidList.remove(server.arg("num").toInt());
  server.send( 200, "text/json", "true");
}

void randomSSID() {
  ssidList._random();
  server.send( 200, "text/json", "true");
}

void clearSSID() {
  ssidList.clear();
  server.send( 200, "text/json", "true");
}

void resetSSID() {
  ssidList.load();
  server.send( 200, "text/json", "true");
}

void saveSSID() {
  ssidList.save();
  server.send( 200, "text/json", "true");
}

void restartESP() {
  server.send( 200, "text/json", "true");
  ESP.reset();
}

//==========Settings==========
void getSettings() {
  server.send ( 200, "text/json", settings.get() );
}

void saveSettings() {
  if (server.hasArg("ssid")) settings.ssid = server.arg("ssid");
  if (server.hasArg("ssidHidden")) {
    if (server.arg("ssidHidden") == "false") settings.ssidHidden = false;
    else settings.ssidHidden = true;
  }
  if (server.hasArg("password")) settings.password = server.arg("password");
  if (server.hasArg("apChannel")) {
    if(server.arg("apChannel").toInt() >= 1 && server.arg("apChannel").toInt() <= 11){
      settings.apChannel = server.arg("apChannel").toInt();
    }
  }
  if (server.hasArg("ssidEnc")) {
    if (server.arg("ssidEnc") == "false") settings.attackEncrypted = false;
    else settings.attackEncrypted = true;
  }
  if (server.hasArg("scanTime")) settings.clientScanTime = server.arg("scanTime").toInt();
  if (server.hasArg("timeout")) settings.attackTimeout = server.arg("timeout").toInt();
  if (server.hasArg("deauthReason")) settings.deauthReason = server.arg("deauthReason").toInt();
  if (server.hasArg("packetRate")) settings.attackPacketRate = server.arg("packetRate").toInt();
  if (server.hasArg("apScanHidden")) {
    if (server.arg("apScanHidden") == "false") settings.apScanHidden = false;
    else settings.apScanHidden = true;
  }
  if (server.hasArg("useLed")) {
    if (server.arg("useLed") == "false") settings.useLed = false;
    else settings.useLed = true;
    attack.refreshLed();
  }
  if (server.hasArg("channelHop")) {
    if (server.arg("channelHop") == "false") settings.channelHop = false;
    else settings.channelHop = true;
  }

  settings.save();
  server.send( 200, "text/json", "true" );
}

void resetSettings() {
  settings.reset();
  server.send( 200, "text/json", "true" );
}

void deleteName() {
  if (server.hasArg("num")) {
    int _num = server.arg("num").toInt();
    nameList.remove(_num);
    server.send( 200, "text/json", "true");
  }
}

void clearNameList() {
  nameList.clear();
  server.send( 200, "text/json", "true" );
}

void editClientName() {
  if (server.hasArg("id") && server.hasArg("name")) {
    nameList.edit(server.arg("id").toInt(), server.arg("name"));
    server.send( 200, "text/json", "true");
  }
}

