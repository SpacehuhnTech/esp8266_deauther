#ifndef WifiManager_h
#define WifiManager_h

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
extern "C" {
#include "user_interface.h"
}

#define WIFI_MODE_OFF 0
#define WIFI_MODE_AP 1
#define WIFI_MODE_STATION 2

/*
   This file contains all necessary functions for hosting and connecting to an access point.
   For compatibility and simplicity, all those functions are global.
 */

// Important strings
const char W_DEAUTHER[] PROGMEM = "deauth.me"; // captive portal domain (alternative to 192.168.4.1)
const char W_WEBINTERFACE[] PROGMEM = "/web";  // default folder containing the web files
const char W_ERROR_PASSWORD[] PROGMEM = "ERROR: Password must have at least 8 characters!";
const char W_DEFAULT_LANG[] PROGMEM = "/lang/default.lang";

const char W_HTML[] PROGMEM = "text/html";
const char W_CSS[] PROGMEM = "text/css";
const char W_JS[] PROGMEM = "application/javascript";
const char W_PNG[] PROGMEM = "image/png";
const char W_GIF[] PROGMEM = "image/gif";
const char W_JPG[] PROGMEM = "image/jpeg";
const char W_ICON[] PROGMEM = "image/x-icon";
const char W_XML[] PROGMEM = "text/xml";
const char W_XPDF[] PROGMEM = "application/x-pdf";
const char W_XZIP[] PROGMEM = "application/x-zip";
const char W_GZIP[] PROGMEM = "application/x-gzip";
const char W_JSON[] PROGMEM = "application/json";
const char W_TXT[] PROGMEM = "text/plain";

const char W_DOT_HTM[] PROGMEM = ".htm";
const char W_DOT_HTML[] PROGMEM = ".html";
const char W_DOT_CSS[] PROGMEM = ".css";
const char W_DOT_JS[] PROGMEM = ".js";
const char W_DOT_PNG[] PROGMEM = ".png";
const char W_DOT_GIF[] PROGMEM = ".gif";
const char W_DOT_JPG[] PROGMEM = ".jpg";
const char W_DOT_ICON[] PROGMEM = ".ico";
const char W_DOT_XML[] PROGMEM = ".xml";
const char W_DOT_PDF[] PROGMEM = ".pdf";
const char W_DOT_ZIP[] PROGMEM = ".zip";
const char W_DOT_GZIP[] PROGMEM = ".gz";
const char W_DOT_JSON[] PROGMEM = ".json";

// Server and other global objects
ESP8266WebServer server(80);
DNSServer dnsServer;
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
File fsUploadFile;

// current WiFi mode and config
uint8_t wifiMode = WIFI_MODE_OFF;

bool   wifi_config_hidden        = false;
bool   wifi_config_captivePortal = false;
String wifi_config_ssid;
String wifi_config_password;
String wifi_config_path;

void stopAP() {
    if (wifiMode == WIFI_MODE_AP) {
        wifi_promiscuous_enable(0);
        WiFi.persistent(false);
        WiFi.disconnect(true);
        wifi_set_opmode(STATION_MODE);
        prntln(W_STOPPED_AP);
        wifiMode = WIFI_MODE_STATION;
    }
}

void wifiUpdate() {
    if ((wifiMode != WIFI_MODE_OFF) && !scan.isScanning()) {
        server.handleClient();
        dnsServer.processNextRequest();
    }
}

String getWifiMode() {
    switch (wifiMode) {
    case WIFI_MODE_OFF:
        return W_MODE_OFF;

        break;

    case WIFI_MODE_AP:
        return W_MODE_AP;

        break;

    case WIFI_MODE_STATION:
        return W_MODE_ST;

        break;

    default:
        return String();
    }
}

String getContentType(String filename) {
    if (server.hasArg("download")) return String(F("application/octet-stream"));

    if (filename.endsWith(str(W_DOT_GZIP))) filename = filename.substring(0, filename.length() - 3);

    if (filename.endsWith(str(W_DOT_HTM))) return str(W_HTML);

    if (filename.endsWith(str(W_DOT_HTML))) return str(W_HTML);

    if (filename.endsWith(str(W_DOT_CSS))) return str(W_CSS);

    if (filename.endsWith(str(W_DOT_JS))) return str(W_JS);

    if (filename.endsWith(str(W_DOT_PNG))) return str(W_PNG);

    if (filename.endsWith(str(W_DOT_GIF))) return str(W_GIF);

    if (filename.endsWith(str(W_DOT_JPG))) return str(W_JPG);

    if (filename.endsWith(str(W_DOT_ICON))) return str(W_ICON);

    if (filename.endsWith(str(W_DOT_XML))) return str(W_XML);

    if (filename.endsWith(str(W_DOT_PDF))) return str(W_XPDF);

    if (filename.endsWith(str(W_DOT_ZIP))) return str(W_XZIP);

    if (filename.endsWith(str(W_DOT_JSON))) return str(W_JSON);

    return str(W_TXT);
}

bool handleFileRead(String path) {
    prnt(W_AP_REQUEST);
    prnt(path);

    if (!path.charAt(0) == SLASH) path = String(SLASH) + path;

    if (path.charAt(path.length() - 1) == SLASH) path += String(F("index.html"));

    String contentType = getContentType(path);

    if (!SPIFFS.exists(path)) {
        if (SPIFFS.exists(path + str(W_DOT_GZIP))) path += str(W_DOT_GZIP);
        else if (SPIFFS.exists(wifi_config_path + path)) path = wifi_config_path + path;
        else if (SPIFFS.exists(wifi_config_path + path + str(W_DOT_GZIP))) path = wifi_config_path + path + str(
                W_DOT_GZIP);
        else {
            prntln(W_NOT_FOUND);
            return false;
        }
    }

    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    prnt(SPACE);
    prntln(W_OK);

    return true;
}

/*
   void handleFileUpload() {
   // only allow uploads on /edit address
   if (server.uri() != "/edit")
    return;

   HTTPUpload& upload = server.upload();
   if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.charAt(0) == SLASH) filename = String(SLASH) + filename;
    //Serial.print("handleFileUpload Name: ");
    //Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
   } else if (upload.status == UPLOAD_FILE_WRITE) {
    //Serial.print("handleFileUpload Data: ");
    //Serial.println(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
   } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
    //Serial.print("handleFileUpload Size: ");
    //Serial.println(upload.totalSize);
   }
   }*/

/*
   void handleFileDelete() {
   if (server.args() == 0)
    return server.send(500, str(W_TXT), str(W_BAD_ARGS));

   String path = server.arg(0);
   //Serial.println("handleFileDelete: " + path);
   if (path == String(SLASH))
    return server.send(500, str(W_TXT), str(W_BAD_PATH));
   if (!SPIFFS.exists(path))
    return server.send(404, str(W_TXT), str(W_FILE_NOT_FOUND));
   SPIFFS.remove(path);
   server.send(200, str(W_TXT), String());
   }*/

/*
   void handleFileCreate() {
   if (server.args() == 0)
    return server.send(500, str(W_TXT), str(W_BAD_ARGS));

   String path = server.arg(0);
   //Serial.println("handleFileCreate: " + path);
   if (path == String(SLASH))
    return server.send(500, str(W_TXT), str(W_BAD_PATH));
   if (SPIFFS.exists(path))
    return server.send(500, str(W_TXT), "FILE EXISTS");
   File file = SPIFFS.open(path, "w");
   if (file)
    file.close();
   else
    return server.send(500, str(W_TXT), "CREATE FAILED");
   server.send(200, str(W_TXT), "");
   }*/
void handleFileList() {
    if (!server.hasArg("dir")) {
        server.send(500, str(W_TXT), str(W_BAD_ARGS));
        return;
    }

    String path = server.arg("dir");
    // Serial.println("handleFileList: " + path);
    Dir dir = SPIFFS.openDir(path);

    String output = String(OPEN_BRACKET); // {
    File   entry;
    bool   first = true;

    while (dir.next()) {
        entry = dir.openFile("r");

        if (first) first = false;
        else output += COMMA;                                                 // ,

        output += OPEN_BRACKET;                                               // [
        output += String(DOUBLEQUOTES) + entry.name() + String(DOUBLEQUOTES); // "filename"
        output += CLOSE_BRACKET;                                              // ]

        entry.close();
    }

    output += CLOSE_BRACKET;
    server.send(200, str(W_JSON).c_str(), output);
}

void sendProgmem(const char* ptr, size_t size, const char* type) {
    server.sendHeader("Content-Encoding", "gzip");
    server.sendHeader("Cache-Control", "max-age=86400");
    server.send_P(200, str(type).c_str(), ptr, size);
}

// path = folder of web files, ssid = name of network, password = password ("0" => no password), hidden = if the network
// is visible, captivePortal = enable a captive portal
void startAP(String path, String ssid, String password, uint8_t ch, bool hidden, bool captivePortal) {
    if (password.length() < 8) {
        prntln(W_ERROR_PASSWORD);
        return;
    }

    if (!path.charAt(0) == SLASH) path = String(SLASH) + path;

    if (password == String(ZERO)) password = String(NEWLINE);

    wifi_config_path     = path;
    wifi_config_ssid     = ssid;
    wifi_config_password = password;
    setWifiChannel(ch);
    wifi_config_hidden        = hidden;
    wifi_config_captivePortal = captivePortal;

    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP(ssid.c_str(), password.c_str(), wifi_channel, hidden);

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, String(ASTERIX), apIP);

    MDNS.begin(str(W_DEAUTHER).c_str());

    server.on(String(F("/list")).c_str(), HTTP_GET, handleFileList); // list directory


    // ================================================================
    // post here the output of the webConverter.py

    if (!settings.getWebSpiffs()) {
        server.on(String(SLASH).c_str(), HTTP_GET, []() {
            sendProgmem(indexhtml, sizeof(indexhtml), W_HTML);
        });
        server.on(String(F("/attack.html")).c_str(), HTTP_GET, []() {
            sendProgmem(attackhtml, sizeof(attackhtml), W_HTML);
        });
        server.on(String(F("/index.html")).c_str(), HTTP_GET, []() {
            sendProgmem(indexhtml, sizeof(indexhtml), W_HTML);
        });
        server.on(String(F("/info.html")).c_str(), HTTP_GET, []() {
            sendProgmem(infohtml, sizeof(infohtml), W_HTML);
        });
        server.on(String(F("/scan.html")).c_str(), HTTP_GET, []() {
            sendProgmem(scanhtml, sizeof(scanhtml), W_HTML);
        });
        server.on(String(F("/settings.html")).c_str(), HTTP_GET, []() {
            sendProgmem(settingshtml, sizeof(settingshtml), W_HTML);
        });
        server.on(String(F("/ssids.html")).c_str(), HTTP_GET, []() {
            sendProgmem(ssidshtml, sizeof(ssidshtml), W_HTML);
        });
        server.on(String(F("/style.css")).c_str(), HTTP_GET, []() {
            sendProgmem(stylecss, sizeof(stylecss), W_CSS);
        });
        server.on(String(F("/js/attack.js")).c_str(), HTTP_GET, []() {
            sendProgmem(attackjs, sizeof(attackjs), W_JS);
        });
        server.on(String(F("/js/scan.js")).c_str(), HTTP_GET, []() {
            sendProgmem(scanjs, sizeof(scanjs), W_JS);
        });
        server.on(String(F("/js/settings.js")).c_str(), HTTP_GET, []() {
            sendProgmem(settingsjs, sizeof(settingsjs), W_JS);
        });
        server.on(String(F("/js/site.js")).c_str(), HTTP_GET, []() {
            sendProgmem(sitejs, sizeof(sitejs), W_JS);
        });
        server.on(String(F("/js/ssids.js")).c_str(), HTTP_GET, []() {
            sendProgmem(ssidsjs, sizeof(ssidsjs), W_JS);
        });
        server.on(String(F("/lang/cn.lang")).c_str(), HTTP_GET, []() {
            sendProgmem(cnlang, sizeof(cnlang), W_JSON);
        });
        server.on(String(F("/lang/cs.lang")).c_str(), HTTP_GET, []() {
            sendProgmem(cslang, sizeof(cslang), W_JSON);
        });
        server.on(String(F("/lang/de.lang")).c_str(), HTTP_GET, []() {
            sendProgmem(delang, sizeof(delang), W_JSON);
        });
        server.on(String(F("/lang/en.lang")).c_str(), HTTP_GET, []() {
            sendProgmem(enlang, sizeof(enlang), W_JSON);
        });
        server.on(String(F("/lang/fr.lang")).c_str(), HTTP_GET, []() {
            sendProgmem(frlang, sizeof(frlang), W_JSON);
        });
        server.on(String(F("/lang/it.lang")).c_str(), HTTP_GET, []() {
            sendProgmem(itlang, sizeof(itlang), W_JSON);
        });
        server.on(String(F("/lang/ru.lang")).c_str(), HTTP_GET, []() {
            sendProgmem(rulang, sizeof(rulang), W_JSON);
        });
        server.on(String(F("/lang/tlh.lang")).c_str(), HTTP_GET, []() {
            sendProgmem(tlhlang, sizeof(tlhlang), W_JSON);
        });
    }
    server.on(str(W_DEFAULT_LANG).c_str(), HTTP_GET, []() {
        if (!settings.getWebSpiffs()) {
            if (settings.getLang() == String(F("cn"))) sendProgmem(cnlang, sizeof(cnlang), W_JSON);
            else if (settings.getLang() == String(F("cs"))) sendProgmem(cslang, sizeof(cslang), W_JSON);
            else if (settings.getLang() == String(F("de"))) sendProgmem(delang, sizeof(delang), W_JSON);
            else if (settings.getLang() == String(F("en"))) sendProgmem(enlang, sizeof(enlang), W_JSON);
            else if (settings.getLang() == String(F("fr"))) sendProgmem(frlang, sizeof(frlang), W_JSON);
            else if (settings.getLang() == String(F("it"))) sendProgmem(itlang, sizeof(itlang), W_JSON);
            else if (settings.getLang() == String(F("ru"))) sendProgmem(rulang, sizeof(rulang), W_JSON);
            else if (settings.getLang() == String(F("tlh"))) sendProgmem(tlhlang, sizeof(tlhlang), W_JSON);

            else handleFileRead(String(F("/web/lang/")) + settings.getLang() + String(F(".lang")));
        } else {
            handleFileRead(String(F("/web/lang/")) + settings.getLang() + String(F(".lang")));
        }
    });

    // ================================================================

    server.on(String(F("/run")).c_str(), HTTP_GET, []() {
        server.send(200, str(W_TXT), str(W_OK).c_str());
        serialInterface.runCommands(server.arg("cmd"));
    });

    server.on(String(F("/attack.json")).c_str(), HTTP_GET, []() {
        server.send(200, str(W_JSON), attack.getStatusJSON());
    });

    /*
       //load editor
       server.on("/edit", HTTP_GET, [](){
       if(!handleFileRead("/edit.htm")) server.send(404, str(W_TXT), "FileNotFound");
       });

       server.on("/edit", HTTP_PUT, handleFileCreate); //create file
       server.on("/edit", HTTP_DELETE, handleFileDelete); //delete file

       //first callback is called after the request has ended with all parsed arguments
       //second callback handles file uploads at that location
       server.on("/edit", HTTP_POST, [](){
       server.send(200, str(W_TXT), "");
       }, handleFileUpload);
     */

    // aggressively caching static assets
    server.serveStatic("/js", SPIFFS, String(wifi_config_path + "/js").c_str(), "max-age=86400");

    // called when the url is not defined here
    // use it to load content from SPIFFS
    server.onNotFound([]() {
        if (!handleFileRead(server.uri())) {
            server.send(404, str(W_TXT), str(W_FILE_NOT_FOUND));
        }
    });

    server.begin();
    wifiMode = WIFI_MODE_AP;

    prntln(W_STARTED_AP);
    printWifiStatus();
}

void printWifiStatus() {
    prnt(String(F("[WiFi] Path: '")));
    prnt(wifi_config_path);
    prnt(String(F("', Mode: '")));

    switch (wifiMode) {
    case WIFI_MODE_OFF:
        prnt(W_MODE_OFF);
        break;

    case WIFI_MODE_AP:
        prnt(W_AP);
        break;

    case WIFI_MODE_STATION:
        prnt(W_STATION);
        break;
    }
    prnt(String(F("', SSID: '")));
    prnt(wifi_config_ssid);
    prnt(String(F("', password: '")));
    prnt(wifi_config_password);
    prnt(String(F("', channel: '")));
    prnt(wifi_channel);
    prnt(String(F("', hidden: ")));
    prnt(b2s(wifi_config_hidden));
    prnt(String(F(", captive-portal: ")));
    prntln(b2s(wifi_config_captivePortal));
}

void startAP() {
    startAP(wifi_config_path.c_str(), wifi_config_ssid.c_str(),
            wifi_config_password.c_str(), wifi_channel, wifi_config_hidden, wifi_config_captivePortal);
}

void startAP(String path) {
    wifi_config_path = path;
    startAP();
}

void loadWifiConfigDefaults() {
    wifi_config_hidden        = settings.getHidden();
    wifi_config_ssid          = settings.getSSID();
    wifi_config_password      = settings.getPassword();
    wifi_config_captivePortal = settings.getCaptivePortal();
    wifi_config_path          = str(W_WEBINTERFACE);
}

void resumeAP() {
    if (wifiMode != WIFI_MODE_AP) {
        wifiMode = WIFI_MODE_AP;
        wifi_promiscuous_enable(0);
        WiFi.softAPConfig(apIP, apIP, netMsk);
        WiFi.softAP(wifi_config_ssid.c_str(), wifi_config_password.c_str(), wifi_channel, wifi_config_hidden);
        prntln(W_STARTED_AP);
    }
}

#endif // ifndef WifiManager_h