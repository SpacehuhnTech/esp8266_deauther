#ifndef functions_h
#define functions_h

#include "Arduino.h"
#include <FS.h>
extern "C" {
  #include "user_interface.h"
}
#include "ArduinoJson.h"

/*
   Here is a collection of useful functions and variables.
   They are used globally via an 'extern' reference in every class.
   Making everything static will lead to problems with the Arduino ESP8266 2.0.0 SDK,
   there were some fixed in later version but we need to use the old version for injecting deauth packets.
 */

uint8_t broadcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t wifi_channel = 1;

// ===== UTF8 FIX ===== //
String escape(String str) {
    str.replace(String(BACKSLASH), String(BACKSLASH) + String(BACKSLASH));
    str.replace(String(DOUBLEQUOTES), String(BACKSLASH) + String(DOUBLEQUOTES));
    return str;
}

bool ascii(char c) {
    return c >= 0 && c <= 127;
}

bool printableAscii(char c) {
    return c >= 32 && c <= 126;
}

bool getBit(uint8_t b, uint8_t n) {
    return (b >> n) % 2 != 0;
}

uint8_t utf8(uint8_t c) {
    if (!getBit(c, 7)) return 1;

    if (getBit(c, 7) && getBit(c, 6) && !getBit(c, 5)) return 2;

    if (getBit(c, 7) && getBit(c, 6) && getBit(c, 5) && !getBit(c, 4)) return 3;

    if (getBit(c, 7) && getBit(c, 6) && getBit(c, 5) && getBit(c, 4) && !getBit(c, 3)) return 4;

    return 0;
}

bool utf8Part(uint8_t c) {
    return getBit(c, 7) && !getBit(c, 6);
}

String fixUtf8(String str) {
    int size = str.length();

    String  result = String();
    char    c;
    uint8_t len;
    bool    ok;

    for (int i = 0; i < size; i++) {
        c   = str.charAt(i);       // get character
        len = utf8(c);             // get utf8 char len

        if (len <= 1) {
            result += c;           // when 1 byte char, add it :)
        }
        else if (i + len > size) { // when char bigger than remaining string, end loop
            i = size + 1;
        }
        else {
            ok = true;

            for (int j = 1; j < len && ok; j++) {
                ok = utf8Part(str.charAt(i + j));   // if following char is compliant or not
            }

            if (ok) result += c;                    // everything is ok, add char and continue
            else {                                  // utf8 char is broken
                for (int j = 1; j < len; j++) {     // go through the next bytes
                    c = str.charAt(i + j);

                    if (utf8(c) == 1) result += c;  // when byte is ascii, add it :)
                }
                i += len - 1;                       // skip utf8 char because we already managed it
            }
        }
    }
    return result;
}

String removeUtf8(String str) {
    str = fixUtf8(str); // fix it in case a utf char is broken
    int size = str.length();

    String  result = String();
    char    c;
    uint8_t len;

    for (int i = 0; i < size; i++) {
        c   = str.charAt(i);        // get character
        len = utf8(c);              // get utf8 char len

        if (len <= 1) result += c;  // when 1 byte char, add it :)
        else i += len - 1;          // skip other chars
    }

    return result;
}

int utf8Len(String str) {
    int size = str.length();

    int  result = 0;
    char c;
    uint8_t len;

    for (int i = 0; i < size; i++) {
        c   = str.charAt(i);     // get character
        len = utf8(c);           // get utf8 char len

        if (len <= 1) result++;  // when 1 byte char, add 1 :)
        else {
            result++;

            for (int j = 1; j < len; j++) {
                c = str.charAt(i + j);

                if (!utf8Part(c) && (utf8(c) == 1)) {
                    Serial.println(c, HEX);
                    result++; // if following char is compliant or not
                }
            }
            i += len - 1;
        }
    }

    return result;
}

String replaceUtf8(String str, String r) {
    str = fixUtf8(str); // fix it in case a utf char is broken
    int size = str.length();

    String  result = String();
    char    c;
    uint8_t len;

    for (int i = 0; i < size; i++) {
        c   = str.charAt(i);        // get character
        len = utf8(c);              // get utf8 char len

        if (len <= 1) result += c;  // when 1 byte char, add it :)
        else {
            result += r;
            i      += len - 1;      // skip other chars
        }
    }

    return result;
}

// ===== LANGUAGE STRING FUNCTIONS ===== //

// for reading Strings from the PROGMEM
String str(const char* ptr) {
    char keyword[strlen_P(ptr)];

    strcpy_P(keyword, ptr);
    return String(keyword);
}

// for converting keywords
String keyword(const char* keywordPtr) {
    char keyword[strlen_P(keywordPtr)];

    strcpy_P(keyword, keywordPtr);

    String  str = "";
    uint8_t len = strlen(keyword);
    uint8_t i   = 0;

    while (i < len && keyword[i] != SLASH && keyword[i] != COMMA) {
        str += keyword[i];
        i++;
    }

    return str;
}

// equals function
bool eqls(const char* str, const char* keywordPtr) {
    if (strlen(str) > 255) return false;  // when string too long

    char keyword[strlen_P(keywordPtr) + 1];
    strcpy_P(keyword, keywordPtr);

    uint8_t lenStr     = strlen(str);
    uint8_t lenKeyword = strlen(keyword);

    if (lenStr > lenKeyword) return false;  // string can't be longer than keyword (but can be smaller because of '/'

    // and ',')

    uint8_t a      = 0;
    uint8_t b      = 0;
    bool    result = true;

    while (a < lenStr && b < lenKeyword) {
        if ((keyword[b] == SLASH) || (keyword[b] == COMMA)) b++;

        if (tolower(str[a]) != tolower(keyword[b])) result = false;

        if (((a == lenStr) && !result) || !result) { // fast forward to next comma
            while (b < lenKeyword && keyword[b] != COMMA) b++;
            result = true;
            a      = 0;
        } else {
            a++;
            b++;
        }
    }
    // comparison correct AND string checked until the end AND keyword checked until the end
    return result && a == lenStr && (keyword[b] == COMMA || keyword[b] == SLASH || keyword[b] == ENDOFLINE);
}

bool eqls(String str, const char* keywordPtr) {
    return eqls(str.c_str(), keywordPtr);
}

// boolean to string
String b2s(bool input) {
    return str(input ? STR_TRUE : STR_FALSE);
}

// boolean to asterix *
String b2a(bool input) {
    return input ? String(ASTERIX) : String(SPACE);
}

// string to boolean
bool s2b(String input) {
    return eqls(input, STR_TRUE);
}

// ===== PRINT FUNCTIONS ===== //
void prnt(const String s) {
    Serial.print(s);
}

void prnt(const bool b) {
    Serial.print(b2s(b));
}

void prnt(const char c) {
    Serial.print(c);
}

void prnt(const char* ptr) {
    Serial.print(FPSTR(ptr));
}

void prnt(const char* ptr, int len) {
    for (int i = 0; i<len; i++) prnt(ptr[i]);
}

void prnt(const int i) {
    Serial.print((String)i);
}

void prnt(const uint32_t i) {
    Serial.printf("%u", i);
}

void prntln() {
    Serial.println();
}

void prntln(const String s) {
    Serial.println(s);
}

void prntln(const bool b) {
    Serial.println(b2s(b));
}

void prntln(const char c) {
    Serial.println(c);
}

void prntln(const char* ptr) {
    Serial.println(FPSTR(ptr));
}

void prntln(const char* ptr, int len) {
    for (int i = 0; i<len; i++) prnt(ptr[i]);
    prntln();
}

void prntln(const int i) {
    Serial.println((String)i);
}

void prntln(const uint32_t i) {
    Serial.printf("%u\r\n", i);
}

/* ===== WiFi ===== */
void setWifiChannel(uint8_t ch) {
    if ((ch != wifi_channel) && (ch > 0) && (ch < 15)) {
        wifi_channel = ch;
        wifi_set_channel(wifi_channel);
    }
}

void setOutputPower(float dBm) {
    if (dBm > 20.5) {
        dBm = 20.5;
    } else if (dBm < 0) {
        dBm = 0;
    }

    uint8_t val = (dBm * 4.0f);
    system_phy_set_max_tpw(val);
}

/* ===== MAC ADDRESSES ===== */
bool macBroadcast(uint8_t* mac) {
    for (uint8_t i = 0; i < 6; i++)
        if (mac[i] != broadcast[i]) return false;

    return true;
}

bool macValid(uint8_t* mac) {
    for (uint8_t i = 0; i < 6; i++)
        if (mac[i] != 0x00) return true;

    return false;
}

bool macMulticast(uint8_t* mac) {
    // see https://en.wikipedia.org/wiki/Multicast_address
    if ((mac[0] == 0x33) && (mac[1] == 0x33)) return true;

    if ((mac[0] == 0x01) && (mac[1] == 0x80) && (mac[2] == 0xC2)) return true;

    if ((mac[0] == 0x01) && (mac[1] == 0x00) && ((mac[2] == 0x5E) || (mac[2] == 0x0C))) return true;

    if ((mac[0] == 0x01) && (mac[1] == 0x0C) && (mac[2] == 0xCD) &&
        ((mac[3] == 0x01) || (mac[3] == 0x02) || (mac[3] == 0x04)) &&
        ((mac[4] == 0x00) || (mac[4] == 0x01))) return true;

    if ((mac[0] == 0x01) && (mac[1] == 0x00) && (mac[2] == 0x0C) && (mac[3] == 0xCC) && (mac[4] == 0xCC) &&
        ((mac[5] == 0xCC) || (mac[5] == 0xCD))) return true;

    if ((mac[0] == 0x01) && (mac[1] == 0x1B) && (mac[2] == 0x19) && (mac[3] == 0x00) && (mac[4] == 0x00) &&
        (mac[5] == 0x00)) return true;

    return false;
}

/* ===== VENDOR LIST (oui.h) ===== */
void getRandomMac(uint8_t* mac) {
    int num = random(sizeof(data_vendors) / 11 - 1);
    uint8_t i;

    for (i = 0; i < 3; i++) mac[i] = pgm_read_byte_near(data_macs + num * 5 + i);

    for (i = 3; i < 6; i++) mac[i] = random(256);
}

int binSearchVendors(uint8_t* searchBytes, int lowerEnd, int upperEnd) {
    uint8_t listBytes[3];
    int     res;
    int     mid = (lowerEnd + upperEnd) / 2;

    while (lowerEnd <= upperEnd) {
        listBytes[0] = pgm_read_byte_near(data_macs + mid * 5);
        listBytes[1] = pgm_read_byte_near(data_macs + mid * 5 + 1);
        listBytes[2] = pgm_read_byte_near(data_macs + mid * 5 + 2);

        res = memcmp(searchBytes, listBytes, 3);

        if (res == 0) {
            return mid;
        } else if (res < 0) {
            upperEnd = mid - 1;
            mid      = (lowerEnd + upperEnd) / 2;
        } else if (res > 0) {
            lowerEnd = mid + 1;
            mid      = (lowerEnd + upperEnd) / 2;
        }
    }

    return -1;
}

String searchVendor(uint8_t* mac) {
    String vendorName = String();
    int    pos        = binSearchVendors(mac, 0, sizeof(data_macs) / 5 - 1);
    int    realPos    = pgm_read_byte_near(data_macs + pos * 5 + 3) | pgm_read_byte_near(data_macs + pos * 5 + 4) << 8;

    if (pos >= 0) {
        char tmp;

        for (int i = 0; i < 8; i++) {
            tmp = (char)pgm_read_byte_near(data_vendors + realPos * 8 + i);

            if (tmp != ENDOFLINE) vendorName += tmp;
            tmp += SPACE;
        }
    }

    return vendorName;
}

/* ===== STRING ===== */
String bytesToStr(const uint8_t* b, uint32_t size) {
    String str;

    for (uint32_t i = 0; i < size; i++) {
        if (b[i] < 0x10) str += ZERO;
        str += String(b[i], HEX);

        if (i < size - 1) str += DOUBLEPOINT;
    }
    return str;
}

String macToStr(const uint8_t* mac) {
    return bytesToStr(mac, 6);
}

bool strToMac(String macStr, uint8_t* mac) {
    macStr.replace(String(DOUBLEPOINT), String());  // ":" -> ""
    macStr.replace("0x", String());                 // "0x" -> ""
    macStr.replace(String(COMMA), String());        // "," -> ""
    macStr.replace(String(DOUBLEQUOTES), String()); // "\"" -> ""
    macStr.toUpperCase();

    if (macStr.length() != 12) {
        prntln(F_ERROR_MAC);
        return false;
    }

    for (uint8_t i = 0; i < 6; i++) mac[i] = strtoul((macStr.substring(i * 2, i * 2 + 2)).c_str(), NULL, 16);

    return true;
}

bool strToIP(String ipStr, uint8_t* ip) {
    String parts[4]  = { "0", "0", "0", "0" };
    int    ipAddr[4] = { -1, -1, -1, -1 };

    int j = 0;

    for (int i = 0; i<ipStr.length(); i++) {
        if (ipStr[i] == '.') j++;
        else parts[j] += ipStr[i];
    }

    for (int i = 0; i<4; i++) {
        ipAddr[i] = parts[i].toInt();
        if ((ipAddr[i] < 0) || (ipAddr[i] > 255)) return false;
    }

    for (int i = 0; i<4; i++) {
        ip[i] = (uint8_t)ipAddr[i];
    }

    return true;
}

void strToColor(String str, uint8_t* buf) {
    str.replace(":", "");
    str.replace("0x", "");
    str.replace(",", "");
    str.replace("#", "");
    str.toUpperCase();

    if (str.length() != 6) {
        prntln(F_COLOR_INVALID);
        return;
    }

    for (uint8_t i = 0; i < 3; i++) buf[i] = strtoul((str.substring(i * 2, i * 2 + 2)).c_str(), NULL, 16);
}

String center(String a, int len) {
    int spaces = len - a.length();

    for (int i = 0; i < spaces; i += 2) {
        a = ' ' + a + ' ';
    }

    a = a.substring(0, len);

    return a;
}

String left(String a, int len) {
    int spaces = len - a.length();

    while (spaces > 0) {
        a = a + ' ';
        spaces--;
    }

    a = a.substring(0, len);

    return a;
}

String right(String a, int len) {
    int spaces = len - a.length();

    while (spaces > 0) {
        a = ' ' + a;
        spaces--;
    }

    a = a.substring(0, len);

    return a;
}

String leftRight(String a, String b, int len) {
    int spaces = len - a.length() - b.length();

    while (spaces > 0) {
        a = a + ' ';
        spaces--;
    }

    a = a + b;

    a = a.substring(0, len);

    return a;
}

/* ===== SPIFFS ===== */
bool progmemToSpiffs(const char* adr, int len, String path) {
    prnt(str(SETUP_COPYING) + path + str(SETUP_PROGMEM_TO_SPIFFS));
    File f = SPIFFS.open(path, "w+");

    if (!f) {
        prntln(SETUP_ERROR);
        return false;
    }

    for (int i = 0; i < len; i++) {
        f.write(pgm_read_byte_near(adr + i));
    }
    f.close();

    prntln(SETUP_OK);

    return true;
}

bool readFile(String path, String& buf) {
    if (path.charAt(0) != SLASH) path = String(SLASH) + path;
    File f = SPIFFS.open(path, "r");

    if (!f) return false;

    if (f.size() == 0) return false;

    while (f.available()) buf += (char)f.read();

    f.close();

    return true;
}

void readFileToSerial(String path, bool showLineNum) {
    if (path.charAt(0) != SLASH) path = String(SLASH) + path;
    File f = SPIFFS.open(path, "r");

    if (!f) {
        prnt(F_ERROR_READING_FILE);
        prntln(path);
        return;
    }

    uint32_t c = 0;
    char     tmp;

    if (showLineNum) {
        prnt(leftRight(String(), (String)c + String(VERTICALBAR), 6));
    }

    while (f.available()) {
        tmp = f.read();
        prnt(tmp);

        if ((tmp == NEWLINE) && showLineNum) {
            c++;
            prnt(leftRight(String(), (String)c + String(VERTICALBAR), 6));
        }
    }

    f.close();
}

bool copyFile(String pathFrom, String pathTo) {
    if (pathFrom.charAt(0) != SLASH) pathFrom = String(SLASH) + pathFrom;

    if (pathTo.charAt(0) != SLASH) pathTo = String(SLASH) + pathTo;

    if (!SPIFFS.exists(pathFrom)) {
        prnt(F_ERROR_FILE);
        prntln(pathFrom);
        return false;
    }

    File f1 = SPIFFS.open(pathFrom, "r");
    File f2 = SPIFFS.open(pathTo, "w+");

    if (!f1 || !f2) return false;

    while (f1.available()) {
        f2.write(f1.read());
    }

    return true;
}

bool renameFile(String pathFrom, String pathTo) {
    if (pathFrom.charAt(0) != SLASH) pathFrom = String(SLASH) + pathFrom;

    if (pathTo.charAt(0) != SLASH) pathTo = String(SLASH) + pathTo;

    if (!SPIFFS.exists(pathFrom)) {
        prnt(F_ERROR_FILE);
        prntln(pathFrom);
        return false;
    }

    SPIFFS.rename(pathFrom, pathTo);
    return true;
}

bool writeFile(String path, String& buf) {
    if (path.charAt(0) != SLASH) path = String(SLASH) + path;
    File f = SPIFFS.open(path, "w+");

    if (!f) return false;

    uint32_t len = buf.length();

    for (uint32_t i = 0; i < len; i++) f.write(buf.charAt(i));
    f.close();

    return true;
}

bool appendFile(String path, String& buf) {
    if (path.charAt(0) != SLASH) path = String(SLASH) + path;
    File f = SPIFFS.open(path, "a+");

    if (!f) return false;

    uint32_t len = buf.length();

    for (uint32_t i = 0; i < len; i++) f.write(buf[i]);
    f.close();

    return true;
}

void checkFile(String path, String data) {
    if (path.charAt(0) != SLASH) path = String(SLASH) + path;

    if (!SPIFFS.exists(path)) writeFile(path, data);
}

bool removeLines(String path, int lineFrom, int lineTo) {
    int  c = 0;
    char tmp;

    if (path.charAt(0) != SLASH) path = String(SLASH) + path;

    String tmpPath = str(F_TMP) + path + str(F_COPY);

    File f  = SPIFFS.open(path, "r");
    File f2 = SPIFFS.open(tmpPath, "w");

    if (!f || !f2) return false;

    while (f.available()) {
        tmp = f.read();

        if ((c < lineFrom) || (c > lineTo)) f2.write(tmp);

        if (tmp == NEWLINE) c++;
    }

    f.close();
    f2.close();
    SPIFFS.remove(path);
    SPIFFS.rename(tmpPath, path);

    return true;
}

bool replaceLine(String path, int line, String& buf) {
    int  c = 0;
    char tmp;

    if (path.charAt(0) != SLASH) path = String(SLASH) + path;

    String tmpPath = "/tmp" + path + "_copy";

    File f  = SPIFFS.open(path, "r");
    File f2 = SPIFFS.open(tmpPath, "w");

    if (!f || !f2) return false;

    while (f.available()) {
        tmp = f.read();

        if (c != line) f2.write(tmp);
        else {
            f2.println(buf);

            while (f.read() != NEWLINE && f.available()) {}
            c++;
        }

        if (tmp == NEWLINE) c++;
    }

    f.close();
    f2.close();
    SPIFFS.remove(path);
    SPIFFS.rename(tmpPath, path);

    return true;
}

JsonVariant parseJSONFile(String path, DynamicJsonBuffer& jsonBuffer) {
    if (path.charAt(0) != SLASH) path = String(SLASH) + path;

    // create JSON Variant
    JsonVariant root;

    // create buffer
    String buf = "";

    // read file into buffer
    if (!readFile(path, buf)) { // if file couldn't be opened, send 404 error
        prnt(F_ERROR_OPEN);
        prntln(path);
        buf = "{}";
    }

    // parse file-buffer into a JSON Variant
    root = jsonBuffer.parse(buf);

    // if parsing unsuccessful
    if (!root.success()) {
        prnt(F_ERROR_PARSING_JSON);
        prntln(path);
        prntln(buf);
    }

    return root;
}

bool removeFile(String path) {
    if (path.charAt(0) != SLASH) path = String(SLASH) + path;
    return SPIFFS.remove(path);
}

void saveJSONFile(String path, JsonObject& root) {
    if (path.charAt(0) != SLASH) path = String(SLASH) + path;

    // create buffer
    String buf;

    // convert JSON object into string and write it into buffer
    root.printTo(buf);

    // if buffer too big
    if (buf.length() > 2048) {
        prntln(F_ERROR_TO_BIG);
        prntln(path);
        prntln(buf);
        return;
    }

    // write buffer into SPIFFS file
    writeFile(path, buf);
}

void saveJSONFile(String path, JsonArray& root) {
    if (path.charAt(0) != SLASH) path = String(SLASH) + path;

    // create buffer
    String buf;

    // convert JSON object into string and write it into buffer
    root.printTo(buf);

    // if buffer too big
    if (buf.length() > 2048) {
        prntln(F_ERROR_TO_BIG);
        prntln(path);
        prntln(buf);
        return;
    }

    // write buffer into SPIFFS file
    writeFile(path, buf);
}

String formatBytes(size_t bytes) {
    if (bytes < 1024) return String(bytes) + "B";
    else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + "KB";
    else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + "MB";
    else return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
}

#endif // ifndef functions_h