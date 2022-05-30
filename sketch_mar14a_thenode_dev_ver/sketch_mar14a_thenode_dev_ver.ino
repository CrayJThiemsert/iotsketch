/* 
 *  Create module for load and write config for "the Node"
 *  
 *  Created Mar 13, 2021 by Jirachai Thiemsert (Cray)
 *  1) Verify config file existing
 *  - If no, create a new config file
 *  - If yes, load config file
 *  
 *  2) Open Wifi mode
 *  - AP mode = to communicate to "the App".
 *  - STA mode = to communicate to "the Cloud Database".
 */


#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h> // Reference: https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html

#include <ESP8266WiFi.h>

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

#define DEFAULT_INTERVAL 10 // Default repeat read sensor in 10 seconds

#define THE_NODE_SSID "theNode_DHT"
#define THE_NODE_PASSWORD "theP455w0rd"

IPAddress ap_local_ip = {192,168,1,144};   // Set up "the Node"'s AP mode IP
IPAddress gateway={192,168,1,1};      // Set up "the Node"'s AP mode Gateway
IPAddress subnet={255,255,255,0};     // Set up "the Node"'s AP mode Subnet

String g_firebaseHost = "";
String g_firebaseAuth = "";
String g_wifiSSID = "";
String g_wifiPassword = "";
String g_workingMode = "";

String g_macAddress = "";

// ******************
WiFiServer wifiServer(80);

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

  Serial.println("------------------");
  // Load config values to global variables
  g_firebaseHost = firebaseHost;
  g_firebaseAuth = firebaseAuth;
  g_wifiSSID = wifiSSID;
  g_wifiPassword = wifiPassword;
  g_workingMode = workingMode;

  Serial.print("Loaded g_firebaseHost: ");
  Serial.println(g_firebaseHost);
  Serial.print("Loaded g_firebaseAuth: ");
  Serial.println(g_firebaseAuth);
  Serial.print("Loaded g_wifiSSID: ");
  Serial.println(g_wifiSSID);
  Serial.print("Loaded g_wifiPassword: ");
  Serial.println(g_wifiPassword);
  Serial.print("Loaded g_workingMode: ");
  Serial.println(g_workingMode);
  
  
  return true;
}

bool clearConfig() {
  return LittleFS.remove("/config.json");
}

bool saveDefaultConfig() {
  StaticJsonDocument<200> doc;
//  doc["serverName"] = "cray.example.com";
//  doc["accessToken"] = "xxx128du9as8du12eoue8da98h123ueh9h98";
  doc["firebaseHost"] = FIREBASE_HOST;
  doc["firebaseAuth"] = FIREBASE_AUTH;
//  doc["wifiSSID"] = WIFI_SSID;
//  doc["wifiPassword"] = WIFI_PASSWORD;
  doc["wifiSSID"] = "";
  doc["wifiPassword"] = "";
  doc["workingMode"] = BURST_MODE;

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, configFile);
  return true;
}

bool turnOnWifi() {
  if(g_wifiSSID == "") {
    // Turn on AP mode only, to talk with "the App" in installation process.
    Serial.println("Turn on AP mode only, to prepare installation process.");
    WiFi.mode(WIFI_AP);
//    WiFi.mode(WIFI_AP_STA);

    WiFi.softAP(THE_NODE_SSID, THE_NODE_PASSWORD); // Set Soft SSID
    WiFi.softAPConfig(ap_local_ip,gateway,subnet); // Set up to module  

    Serial.println(""); 
    Serial.println("WiFi connected");     // Display connected success
    Serial.println("AP IP address: "); 
    Serial.println(WiFi.softAPIP());       // Show ESP8266's IP Addres

    g_macAddress = WiFi.softAPmacAddress().c_str();
    Serial.printf("MAC address String = %s\n", WiFi.softAPmacAddress().c_str());
    Serial.print("g_macAddress: ");
    Serial.println(g_macAddress);

    if(!runWebServer()) {
      Serial.println("Failed to run web server.");
    } else {
      Serial.println("Run web server is successfully.");
    }
    
    
  } else {
    Serial.println("Turn on STA mode only, to prepare internet access process.");
//    WiFi.mode(WIFI_AP_STA);
    WiFi.mode(WIFI_STA);

    // connect to wifi.
    WiFi.begin(g_wifiSSID, g_wifiPassword);
    Serial.print("connecting");
    
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();
    Serial.print("STA connected: ");
    Serial.println(WiFi.localIP());
  }

  /*
   * Wifi Connection status:
   * 0 : WL_IDLE_STATUS when Wi-Fi is in process of changing between statuses
   * 1 : WL_NO_SSID_AVAILin case configured SSID cannot be reached
   * 3 : WL_CONNECTED after successful connection is established
   * 4 : WL_CONNECT_FAILED if connection failed
   * 6 : WL_CONNECT_WRONG_PASSWORD if password is incorrect
   * 7 : WL_DISCONNECTED if module is not configured in station mode
   */
  
  Serial.printf("Connection status: %d\n", WiFi.status());
  
  return true;
}

bool runWebServer() {
  wifiServer.begin();
  
  return true;
}

void webButtonsCommand() {
  // put your main code here, to run repeatedly:
  WiFiClient client = wifiServer.available();
  String command = "";

  if (client) {

    while (client.connected()) {

      while (client.available()>0) {
        char c = client.read();
        if (c == '\n') {
          break;
        }
        command += c;
        Serial.write(c);
      }
      
      if (command == "POWER") {
        
        Serial.println("We got the power !!");
      }else if (command == "TEMPUP") {
        
        Serial.println("We got the power !!");
      }else if (command == "TEMPDOWN") {
        
        Serial.println("We got the power !!");
      }else if (command == "FAN") {
        
        Serial.println("We got the power !!");
      }else if (command == "MODE") {

        Serial.println("We got the power !!");
      }

      command = "";
      delay(10);
    }

    client.stop();
    Serial.println("Client disconnected");
  }
}

// ==================================================================
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
  // Clear config file
  if(!clearConfig()) {
    Serial.println("Failed to clear config file system");
    return;
  } else {
    Serial.println("Clear config file system successfuly!");
  }
  */
  
  /*
   // comment this line because we already create and upload the config.json file to the disk
  if (!saveDefaultConfig()) {
    Serial.println("Failed to save config");
  } else {
    Serial.println("Config saved");
  }
  */
  
  
  if (!loadConfig()) {
    Serial.println("Failed to load config");

    // Create a new config file
    if (!saveDefaultConfig()) {
      Serial.println("Failed to save new config");
    } else {
      Serial.println("New Config saved");
      delay(200);
      
      if (!loadConfig()) {
        Serial.println("Failed to load new config");
      } else {
        Serial.println("New Config loaded");

        if(!turnOnWifi()) {
          Serial.println("Failed to turn on Wifi");
        } else {
          Serial.println("Wifi turn on"); 
        }
      }
    }
  } else {
    Serial.println("Config loaded");

    if(!turnOnWifi()) {
      Serial.println("Failed to turn on Wifi");
    } else {
      Serial.println("Wifi turn on"); 
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  webButtonsCommand();
}
