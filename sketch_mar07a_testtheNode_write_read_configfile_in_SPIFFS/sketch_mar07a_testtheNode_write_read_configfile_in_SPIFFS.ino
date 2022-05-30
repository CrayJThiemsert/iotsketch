/* 
 *  Create module for load and write config for "the Node"
 *  
 *  Created Mar 07, 2021 by Jirachai Thiemsert (Cray)
 */


#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>

// Config Firebase
#define FIREBASE_HOST "asset-management-lff.firebaseio.com"
#define FIREBASE_AUTH "tyPYNCdjHQREFNsW46sha4JhOFG4WG4U7pL8iShx"

//// Config connect WiFi Cray Home
#define WIFI_SSID "Killing Me Softly 2G"
#define WIFI_PASSWORD "14mP455w0rd"

//// Config connect WiFi Intec Office
//#define WIFI_SSID "Intec 02"
//#define WIFI_PASSWORD "solar100"

// Config connect ChangMalayCondo_2
//#define WIFI_SSID "ChangMalayCondo_2"
//#define WIFI_PASSWORD "marktoberdorf"

// Working Mode
#define POLLING_MODE "polling mode" 
#define REQUEST_MODE "request mode" // Default, use when installation process
#define BURST_MODE "burst mode"
#define OFFLINE_MODE "offline mode"

bool loadConfig() {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
    return false;
  }

//  const char* serverName = doc["serverName"];
//  const char* accessToken = doc["accessToken"];

  const char* firebaseHost = doc["firebaseHost"];
  const char* firebaseAuth = doc["firebaseAuth"];
  const char* wifiSSID = doc["wifiSSID"];
  const char* wifiPassword = doc["wifiPassword"];
  const char* workingMode = doc["workingMode"];

  const char* testUnknown = doc["testUnknown"];

  // Real world application would store these values in some variables for
  // later use.

//  Serial.print("Loaded serverName: ");
//  Serial.println(serverName);
//  Serial.print("Loaded accessToken: ");
//  Serial.println(accessToken);

  Serial.print("Loaded firebaseHost: ");
  Serial.println(firebaseHost);
  Serial.print("Loaded firebaseAuth: ");
  Serial.println(firebaseAuth);
  Serial.print("Loaded wifiSSID: ");
  Serial.println(wifiSSID);
  Serial.print("Loaded wifiPassword: ");
  Serial.println(wifiPassword);
  Serial.print("Loaded workingMode: ");
  Serial.println(workingMode);

  Serial.print("Loaded testUnknown: ");
  Serial.println(testUnknown);
  return true;
}

bool saveConfig() {
  StaticJsonDocument<200> doc;
//  doc["serverName"] = "cray.example.com";
//  doc["accessToken"] = "xxx128du9as8du12eoue8da98h123ueh9h98";
  doc["firebaseHost"] = FIREBASE_HOST;
  doc["firebaseAuth"] = FIREBASE_AUTH;
  doc["wifiSSID"] = WIFI_SSID;
  doc["wifiPassword"] = WIFI_PASSWORD;
  doc["workingMode"] = REQUEST_MODE;

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, configFile);
  return true;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("");
  delay(1000);
  Serial.println("Mounting FS...");

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  /*
   // comment this line because we already create and upload the config.json file to the disk
  if (!saveConfig()) {
    Serial.println("Failed to save config");
  } else {
    Serial.println("Config saved");
  }
  */
  

  if (!loadConfig()) {
    Serial.println("Failed to load config");
  } else {
    Serial.println("Config loaded");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
