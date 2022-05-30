/**

   Written and Verified by 
   Mr. Jirachai Thiemsert
   Project: IOT System
   Created: FEB 2021

*/
// To connect Wifi
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// To save local config memory
#include <EEPROM.h>

// To read humid and temperature sensor
#include <DHT.h>

// To update Firebase Realtime Database
//#include<FirebaseArduino.h>
#include<FirebaseESP8266.h>

// To get the current date time from internet
#include <NTPClient.h>
#include <WiFiUdp.h>

// ======================================================================
// Default "theNode" AP Mode SSID and password, use to transfer data between "the App" and "the Node".
#define THE_NODE_SSID "theNode_DHT"
#define THE_NODE_PASSWORD ""
const char* host = "thenode";
IPAddress ap_local_ip = {192,168,1,199};   // Set up "the Node"'s AP mode IP
IPAddress gateway={192,168,1,1};      // Set up "the Node"'s AP mode Gateway
IPAddress subnet={255,255,255,0};     // Set up "the Node"'s AP mode Subnet
// ======================================================================
// Working Mode(mode) [20 characters]
#define POLLING_MODE "polling" 
#define REQUEST_MODE "request" 
#define BURST_MODE "burst" // Default, use after installation process finished
#define OFFLINE_MODE "offline"
#define SETUP_MODE "setup"
// ======================================================================
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

// Bangkok timezone GMT+7
#define BANGKOK_TIMEZONE 25200
// ======================================================================
// Config DHT - Humid and Temperature Sensor
#define DHTPIN 4
//#define DHTTYPE DHT11
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
// ======================================================================
#define FIREBASE_HOST "asset-management-lff.firebaseio.com"
#define FIREBASE_AUTH "tyPYNCdjHQREFNsW46sha4JhOFG4WG4U7pL8iShx"
#define DEFAULT_READING_INTERVAL "10000"
#define DEFAULT_READING_INTERVAL_LONG 10000
// ======================================================================
// Set to true to reset eeprom before to write something
#define RESET_EEPROM true
#define D0_LED_BUILTIN 2
// ======================================================================
// Variables
int i = 0;
int statusCode;
const char* ssid = "text";
const char* passphrase = "text";
String st;
String content;

String gSSID = "";
String gSSIDPassword = "";
String gFirebaseHost = "";
String gFirebaseAuth = "";
String gWorkingMode = BURST_MODE;
String gCurrentDateTimeString = "-";
float gTemperature = 0;
float gHumidity = 0;
String gMacAddress = "";
String gDeviceName = "";
String gDeviceReadingInterval = DEFAULT_READING_INTERVAL;
String gDeviceLocalIp = "";
float gReadVoltage = 0;

// Function Decalration
bool testWifi(void);
void createWebServer(void);
void launchWeb(void);
void setupAP(void);
bool getCurrentDateTime(void);
bool readSensor(void);
bool saveDataToCloudDatabase(void);
//bool createDeviceDataToCloudDatabase(void);
bool updateWorkingMode(String qmode);
bool listenResetButton(long elapsedMS);
bool resetManufacturingMode(void);
void processNormalTasksEngine(unsigned long currentMillis);
bool loadInternalConfig();
bool processReadSensor(void);
float readVoltage(); // read internal VCC
String getDeviceReadingIntervalFromCloudDatabase();
bool writeLocalReadingIntervalConfig();
void initialFirebase();
void processDeepSleep();
// Buttons
const int resetButton = 16;
int gIsResetButtonPress = 0;
int gResetCount = 0;

// Multitask
long led_interval = 1000;
long BurstModeInterval = 10000; // 10 seconds
long lastTime = millis();
long normalProcessLastTime = millis();
bool ledState=false;

unsigned long previousMillis1 = 0;  //store last time LED1 was blinked
//const long period1 = 1000;         // period at which led1 blinks in ms

unsigned long previousMillis2 = 0;  //store last time LED2 was blinked
//const long period2 = 200;             // period at which led1 blinks in ms

//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);

FirebaseData firebaseData;
FirebaseJson firebasebJson;
ADC_MODE(ADC_VCC);  // allows you to monitor the internal VCC level; it varies with WiFi load

void setup() {

  Serial.begin(115200); //Initialising if(DEBUG)Serial Monitor
  // Wait for serial to initialize.
  while(!Serial) { }
  
  Serial.println();
  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  delay(100);
  
  // Initial reset button
  pinMode(resetButton, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D0_LED_BUILTIN, OUTPUT);

  Serial.println();
  Serial.println();
  Serial.println("Startup");

  Serial.print("ESP Board MAC Address:  ");
  gMacAddress = WiFi.macAddress();
  Serial.println(gMacAddress);

  // load configuration values from eeprom
  loadInternalConfig();

  // Try to connect wifi by loaded ssid and password values from eeprom
////  WiFi.begin(esid.c_str(), epass.c_str());
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(gSSID, gSSIDPassword);
  if (testWifi())  {
    Serial.println("Succesfully Internet Wifi Connected!!!");

    // Initial Firebase to ready to get/set cloud database value
    initialFirebase();
    
    // Verify readingInterval value between cloud database and local memory
    String cloudReadingInterval = getDeviceReadingIntervalFromCloudDatabase();
    Serial.print("cloudReadingInterval: ");
    Serial.println(cloudReadingInterval.toInt());
    Serial.print("gDeviceReadingInterval: ");
    Serial.println(gDeviceReadingInterval.toInt());
    Serial.print("cloudReadingInterval.toInt() != gDeviceReadingInterval.toInt()=");
    Serial.println(cloudReadingInterval.toInt() != gDeviceReadingInterval.toInt());
    if(cloudReadingInterval.toInt() != gDeviceReadingInterval.toInt()) {
      Serial.println("");
      Serial.println("[readingInterval] value has updated from cloud database, then we need to update value in eeprom, too.");
      gDeviceReadingInterval = cloudReadingInterval;

      if(!writeLocalReadingIntervalConfig()) {
        Serial.println("writeLocalReadingIntervalConfig is failured!!!");
      }
    } else {
      Serial.println("readingInterval value equals then do nothing");
    }

    // Test read and update readingInterval value from cloud database
    /*
    // launch local webserver to wait for setup request from "theApp".
    launchWeb();
    
    Serial.println();
    Serial.println("Waiting update settings device trigger.");
    */
//    Serial.print("gWorkingMode == SETUP_MODE...=>");
//    Serial.println(strcmp(gWorkingMode, SETUP_MODE) == 0);
//    Serial.println(gWorkingMode.equals(SETUP_MODE));
//    if(strcmp(gWorkingMode, SETUP_MODE) == 0) {
    if(gWorkingMode.equals(SETUP_MODE)) {
      Serial.println("Doing.. setup new device...!!!");
      if(createDeviceDataToCloudDatabase()) {
        Serial.println("Create a new device in cloud database is successfully!!!");
      } else {
        Serial.println("Create a new device in cloud database failured!!!");
      }
      
    } else {
      Serial.println("Continue to sensor reading process...!!!");
    }
    
    // run read sensor once when start process
    if(!processReadSensor()) {
      Serial.println("processReadSensor falied!!");
    }
    
//    while ((WiFi.status() != WL_CONNECTED))  {
//      Serial.println(".");
//      delay(1000); // ** here
//      server.handleClient();
//      MDNS.update();
//    }
    
    processDeepSleep();
    return; // never get in this line
  } else  {
    Serial.println("Turning the HotSpot On");
    setupAP();// Setup HotSpot

    Serial.println();
    Serial.println("Waiting add new device trigger.");
    
    while ((WiFi.status() != WL_CONNECTED))  {
      Serial.println(".");
      delay(1000);
      server.handleClient();
  //    MDNS.update();
    }
  }

  // For test only ------------------------------- start
//  Serial.println("Turning the HotSpot On");
//  setupAP();// Setup HotSpot
//  launchWeb();
  // For test only ------------------------------- end

  

}
void processDeepSleep() {
//  readVoltage();  // read internal VCC
  long readingInterval = strtol(gDeviceReadingInterval.c_str(), NULL, 0 );
   if(readingInterval == 0) {
    readingInterval = DEFAULT_READING_INTERVAL_LONG;
   }
//  ESP.deepSleep(10E6, WAKE_RFCAL); // good night!  D0 fires a reset in 10 seconds...
  unsigned long sleepInterval = readingInterval * 1000;
  Serial.print("I am going to sleep and will wakeup in next ");
  Serial.print(sleepInterval);
  Serial.println("microsecond");
  ESP.deepSleep(sleepInterval, WAKE_RFCAL); // good night!  D0 fires a reset in 10 seconds...
//  ESP.deepSleep(sleepInterval, WAKE_NO_RFCAL); // good night!  D0 fires a reset in 10 seconds...
//  ESP.deepSleep(sleepInterval); // good night!  D0 fires a reset in 10 seconds...
}

void loop() {

  // Test read and update readingInterval value from cloud database
  /*
  
  // calculate elapsedTime
// -------------------------------------------------------------------- new structure     
  unsigned long currentMillis = millis(); // store the current time

  blinkLEDTaskOnce(currentMillis);

  processNormalTasksEngine(currentMillis);

//  // do normal task because intect board have no reset button switch setup. 07-apr-2021 11:50
////  listenResetButton(elapsedTime);

  */
  // -------------------------------------------------------------------- deep sleep structure with 
  
//  long readingInterval = strtol(gDeviceReadingInterval.c_str(), NULL, 0 );
//   if(readingInterval == 0) {
//    readingInterval = DEFAULT_READING_INTERVAL_LONG;
//   }
////  ESP.deepSleep(10E6, WAKE_RFCAL); // good night!  D0 fires a reset in 10 seconds...
//  long sleepInterval = readingInterval * 1000;
//  Serial.print("I am going to sleep and will wakeup in next ");
//  Serial.print(sleepInterval);
//  Serial.println("microsecond");
//  ESP.deepSleep(sleepInterval, WAKE_RFCAL); // good night!  D0 fires a reset in 10 seconds...
  
}

void processNormalTasksEngine(unsigned long currentMillis) {
//    static long ledTime = 0;  // static define value once only
//    ledTime = ledTime + elapsedMS;

//    Serial.print("normal BurstModeInterval=");
//    Serial.println(BurstModeInterval);
    
   long readingInterval = strtol(gDeviceReadingInterval.c_str(), NULL, 0 );
   if(readingInterval == 0) {
    readingInterval = DEFAULT_READING_INTERVAL_LONG;
   }

   if (currentMillis - previousMillis2 >= readingInterval) {    // check if gDeviceReadingInterval ms passed
    previousMillis2 = currentMillis;   // save the last time you run normal process

    if ((WiFi.status() == WL_CONNECTED))  {
        Serial.println("");
        Serial.print("gDeviceReadingInterval(millisecond)=");
        Serial.println(gDeviceReadingInterval);
        Serial.print("previousMillis2=");
        Serial.println(previousMillis2);
        Serial.print("currentMillis=");
        Serial.println(currentMillis);
        Serial.print("readingInterval=");
        Serial.println(readingInterval);
  
        processReadSensor();
  
      }
  }
   
////    if(ledTime >= BurstModeInterval) {
////    10sec=10000ms=600
////    1sec=1000ms=60
////    5sec=5000ms=300
////    readingInterval = 300; = 5sec
//    if(ledTime >= readingInterval) {
//      if ((WiFi.status() == WL_CONNECTED))  {
//        Serial.println("");
//        Serial.print("gDeviceReadingInterval(millisecond)=");
//        Serial.println(gDeviceReadingInterval);
//        Serial.print("elapsedMS=");
//        Serial.println(elapsedMS);
//        Serial.print("normal ledTime=");
//        Serial.println(ledTime);
//        Serial.print("readingInterval=");
//        Serial.println(readingInterval);
//  
//        processReadSensor();
//  
//  //      ledTime = ledTime - BurstModeInterval;
//        ledTime = ledTime - readingInterval;
//      }
//    } 
//    } else  {
//      Serial.print(".");
////      if(elapsedMS == 1) {
//////      Serial.println("not reach interval time yet!!!");
////        Serial.print("normal ledTime=");
////        Serial.println(ledTime);
////        Serial.print("elapsedMS=");
////        Serial.println(elapsedMS);
////        Serial.println("");
////      }
//    }
    
//  }
}

void blinkLEDTaskOnce(unsigned long currentMillis) {
  if (currentMillis - previousMillis1 >= led_interval) {    // check if 1000ms passed
    previousMillis1 = currentMillis;   // save the last time you blinked the LED
    if (ledState == LOW) {  // if the LED is off turn it on and vice-versa
      ledState = HIGH;   //change led state for next iteration
    } else {
      ledState = LOW;
    }
    digitalWrite(D0_LED_BUILTIN, ledState);    //set LED with ledState to blink again

    // listen local webserver every 1 seconds
    server.handleClient();
    MDNS.update();
  }

  
//  static long ledTime = 0;  // static define value once only
//  ledTime = ledTime + elapsedMS;
// 
//  if(ledTime >= led_interval) {
//    ledState = !ledState;
//    digitalWrite(D0_LED_BUILTIN,ledState);   
//    ledTime = ledTime - led_interval;
//
//    // listen local webserver every 1 seconds
//    server.handleClient();
//    MDNS.update();
//  }
}

bool loadInternalConfig() {
  bool result = false;

  EEPROM.begin(512); //Initialasing EEPROM
  delay(100);
  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");

  String esid = "";
  for (int i = 0; i < 32; ++i)  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  gSSID = esid;
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i)  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  gSSIDPassword = epass;
  
  String efbhost = "";
  for (int i = 96; i < 160; ++i)  {
    efbhost += char(EEPROM.read(i));
  }
  Serial.print("FBHOST: ");
  Serial.println(efbhost);
  gFirebaseHost = efbhost;
  if(gFirebaseHost == "") {
    gFirebaseHost = FIREBASE_HOST;
    Serial.print("empty FBHOST then use default: ");
    Serial.println(gFirebaseHost);
  }

  String efbauth = "";
  for (int i = 160; i < 224; ++i)  {
    efbauth += char(EEPROM.read(i));
  }
  Serial.print("FBAUTH: ");
  Serial.println(efbauth);
  gFirebaseAuth = efbauth;
  if(gFirebaseAuth == "") {
    gFirebaseAuth = FIREBASE_AUTH;
    Serial.print("empty FBAUTH then use default: ");
    Serial.println(gFirebaseAuth);
  }

  String emode = "";
  for (int i = 224; i < 244; ++i)  {
    emode += char(EEPROM.read(i));
  }
  Serial.print("MODE: ");
  Serial.println(emode);
  gWorkingMode = emode;

  String ename = "";
  for (int i = 244; i < 308; ++i)  {
    ename += char(EEPROM.read(i));
  }
  Serial.print("NAME: ");
  Serial.println(ename);
  gDeviceName = ename;

  String einterval = "";
  for (int i = 308; i < 318; ++i)  {
    einterval += char(EEPROM.read(i));
  }
  Serial.print("READING INTERVAL: ");
  Serial.println(einterval);
  gDeviceReadingInterval = einterval;
  if(gDeviceReadingInterval == "") {
    gDeviceReadingInterval = DEFAULT_READING_INTERVAL;
    Serial.print("empty READING INTERVAL then use default: ");
    Serial.println(gDeviceReadingInterval);
  }
  
  return result;
}

bool processReadSensor(void) {
  bool result = false;

  Serial.println("doing read sensor...");
  if(readSensor()) {
    Serial.println("Succesfully read sensor data!!!++++++++");
    if(saveDataToCloudDatabase()) {
      Serial.println("Successfully Update data to the Cloud!!!");
      Serial.println("");
      result = true;
        // Delay belong to working mode, default is burst mode, 10 seconds
//        delay(10000);
    } else {
      Serial.println("Update data to the Cloud Failured!!!");
    }  
  } else {
    Serial.println("Read sensor Failured!!!");
  }

  return result;
}


//----------------------------------------------- Functions used for WiFi credentials saving and connecting to it which you do not need to change 
bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 30 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("gWorkingMode=");
      Serial.println(gWorkingMode);
      return true;
    }
    delay(1000);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

// author apicquot from https://forum.arduino.cc/index.php?topic=228884.0
String IpAddress2String(const IPAddress& ipAddress) {
    return String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]);
}

void launchWeb() {
  Serial.println("");
  Serial.println("launchWeb");
//  if (WiFi.status() == WL_CONNECTED) 
//    Serial.println("WiFi connected");
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    String dnsMacAddress = gMacAddress;
    dnsMacAddress.replace(":", "");
    String hostName = host + dnsMacAddress;
    Serial.print("hostName=");
    Serial.println(hostName);
    
    MDNS.begin(hostName); 
    
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
    gDeviceLocalIp = IpAddress2String(WiFi.localIP());
    Serial.print("gDeviceLocalIp: ");
    Serial.println(gDeviceLocalIp);
    Serial.print("SoftAP IP: ");
    Serial.println(WiFi.softAPIP());
    createWebServer();
    
    // Start the server
    server.begin();
    MDNS.addService("http", "tcp", 80);
    Serial.println("WebServer started");
//    Serial.printf("Ready! Open http://%s.local in your browser\n", host);
    Serial.print("Ready! Open http://");
    Serial.print(hostName);
    Serial.println(".local in your browser\n");
  }
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
//  WiFi.softAP("techiesms", "");
//  Serial.println("softap");

  WiFi.softAP(THE_NODE_SSID, THE_NODE_PASSWORD); // Set Soft SSID
  WiFi.softAPConfig(ap_local_ip,gateway,subnet); // Set up to module  
  Serial.println("set softap");
  Serial.print("AP Web Server IP address: "); 
  Serial.println(WiFi.softAPIP());       // Show ESP8266's IP Address

  // start add new device webserver
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();

  Serial.println("WebServer started");
  
}

void createWebServer(void) {
 
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");

      String qfbhost = server.arg("fbhost");
      String qfbauth = server.arg("fbauth");

      String qname = server.arg("name");
      String qmode = server.arg("mode");

      String qinterval = server.arg("interval");

      bool hasValues = false;
      bool wantReset = false;
      
      if (qsid.length() > 0 && qpass.length() > 0) {
        hasValues = true;
        wantReset = true;
        Serial.println("clearing eeprom");
//        for (int i = 0; i < 96; ++i) {  // ssid, password
//        for (int i = 0; i < 224; ++i) { // + firebase host, firabase auth
//        for (int i = 0; i < 244; ++i) { // + working mode
//        for (int i = 0; i < 308; ++i) { // + device name
        for (int i = 0; i < 318; ++i) { // + reading interval
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        // save internet ssid and password into eeprom
        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i) {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i) {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
      } 

      // save firebase host and auth into eeprom
      if (qfbhost.length() > 0 && qfbauth.length() > 0) {
        hasValues = true;
        // fbhost size 64 characters
        // Clear part
        for (int i = 0; i < 64; ++i) { 
          EEPROM.write(96 + i, 0);
        }
        Serial.println("writing eeprom fbhost:");
        for (int i = 0; i < qfbhost.length(); ++i) {
          EEPROM.write(96 + i, qfbhost[i]);
          Serial.print("Wrote: ");
          Serial.println(qfbhost[i]);
        }
        // fbauth size 64 characters
        // Clear part
        for (int i = 0; i < 64; ++i) { 
          EEPROM.write(160 + i, 0);
        }
        Serial.println("writing eeprom fbauth:");
        for (int i = 0; i < qfbauth.length(); ++i) {
          EEPROM.write(160 + i, qfbauth[i]);
          Serial.print("Wrote: ");
          Serial.println(qfbauth[i]);
        }
      }

      // save working mode into eeprom
      if (qmode.length() > 0) {
        hasValues = true;
        // mode size 20 characters
        // Clear part
        for (int i = 0; i < 20; ++i) { 
          EEPROM.write(224 + i, 0);
        }
        Serial.println("writing eeprom mode:");
        for (int i = 0; i < qmode.length(); ++i) {
          EEPROM.write(224 + i, qmode[i]);
          Serial.print("Wrote: ");
          Serial.println(qmode[i]);
        }
      }

      // save device name into eeprom
      if (qname.length() > 0) {
        hasValues = true;
        // (device)name size 64 characters
        // Clear part
        for (int i = 0; i < 64; ++i) { 
          EEPROM.write(244 + i, 0);
        }
        Serial.println("writing eeprom device name:");
        for (int i = 0; i < qname.length(); ++i) {
          EEPROM.write(244 + i, qname[i]);
          Serial.print("Wrote: ");
          Serial.println(qname[i]);
        }
      }

      // save device reading interval into eeprom
      if (qinterval.length() > 0) {
        hasValues = true;
        // (device)reading interval size 10 characters. (1 week = 168 hours = 604,800,000 milliseconds)
        // Clear part
        for (int i = 0; i < 10; ++i) { 
          EEPROM.write(308 + i, 0);
        }
        Serial.println("writing eeprom device reading interval:");
        for (int i = 0; i < qinterval.length(); ++i) {
          EEPROM.write(308 + i, qinterval[i]);
          Serial.print("Wrote: ");
          Serial.println(qinterval[i]);
        }
      }
//      // save firebase host and auth into eeprom
//      if (qfbhost.length() > 0 && qfbauth.length() > 0) {
//        hasValues = true;
//        // fbhost size 64 characters
//        Serial.println("writing eeprom fbhost:");
//        for (int i = 0; i < qfbhost.length(); ++i)
//        {
//          EEPROM.write(96 + i, qfbhost[i]);
//          Serial.print("Wrote: ");
//          Serial.println(qfbhost[i]);
//        }
//        // fbauth size 64 characters
//        Serial.println("writing eeprom fbauth:");
//        for (int i = 0; i < qfbauth.length(); ++i)
//        {
//          EEPROM.write(160 + i, qfbauth[i]);
//          Serial.print("Wrote: ");
//          Serial.println(qfbauth[i]);
//        }
//      }
//
//      // save working mode into eeprom
//      if (qmode.length() > 0) {
//        hasValues = true;
//        // mode size 20 characters
//        Serial.println("writing eeprom mode:");
//        for (int i = 0; i < qmode.length(); ++i)
//        {
//          EEPROM.write(224 + i, qmode[i]);
//          Serial.print("Wrote: ");
//          Serial.println(qmode[i]);
//        }
//      }
//
//      // save device name into eeprom
//      if (qname.length() > 0) {
//        hasValues = true;
//        // (device)name size 64 characters
//        Serial.println("writing eeprom device name:");
//        for (int i = 0; i < qname.length(); ++i) {
//          EEPROM.write(244 + i, qname[i]);
//          Serial.print("Wrote: ");
//          Serial.println(qname[i]);
//        }
//      }
//
//      // save device reading interval into eeprom
//      if (qinterval.length() > 0) {
//        hasValues = true;
//        // (device)reading interval size 10 characters (1 week = 168 hours = 604,800,000 milliseconds)
//        Serial.println("writing eeprom device reading interval:");
//        for (int i = 0; i < qinterval.length(); ++i) {
//          EEPROM.write(308 + i, qinterval[i]);
//          Serial.print("Wrote: ");
//          Serial.println(qinterval[i]);
//        }
//      }
      
      // Test print out interval argument
      if (qinterval.length() > 0) {
        hasValues = true;        
        wantReset = true;
        Serial.print("qinterval=");
        for (int i = 0; i < qinterval.length(); ++i) {
          EEPROM.write(308 + i, qinterval[i]);
          Serial.print("Wrote: ");
          Serial.println(qinterval[i]);
        }
      }
      
      if(hasValues) {
        // submit eeprom saving 
        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        Serial.println("Sending 200");
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", content);
//        server.sendHeader("Access-Control-Allow-Origin", "*");
//        server.addHeader("Content-Type", "text/plain");
//        server.send(statusCode, "application/json", content);
//        server.on("/header", HTTP_GET, [](AsyncWebServerRequest *request){
//          AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Ok");
//          response->addHeader("Test-Header", "My header value");
//          request->send(response);
//        });
        if(wantReset) {
          ESP.reset();  
        } else {
          // reload configuration values from eeprom
          loadInternalConfig();
        }
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(statusCode, "application/json", content);
      }
    });
}
//------------------------- Functions about cloud

bool saveDataToCloudDatabase(void) {
  int c = 0;

//  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
//  gFirebaseHost = FIREBASE_HOST;
//  gFirebaseAuth = FIREBASE_AUTH;
//  Serial.println("Waiting for update cloud");
//  Serial.print("Firebase Host:");
//  Serial.println(gFirebaseHost);
//  Serial.print("Firebase Auth:");
//  Serial.println(gFirebaseAuth);

  readVoltage();  // read internal VCC

  // Get current date time uid.
  if(getCurrentDateTime()) {

    Serial.print("gCurrentDateTimeString=");
    Serial.println(gCurrentDateTimeString);
    // Append a new value to temporary node
//    String nodeString = "users/cray/devices/ht-00001/ht-00001_history/" + gCurrentDateTimeString;
    String nodeString = "users/cray/devices/" + gMacAddress + "/" + gMacAddress + "_history/" + gCurrentDateTimeString;
    Serial.print("nodeString=");
    Serial.println(nodeString);

    // Create FirebaseJson object
//    firebasebJson.clear().add("uid", gCurrentDateTimeString);
//    firebasebJson.add("temperature", gTemperature);
//    firebasebJson.add("humidity", gHumidity);
//    firebasebJson.add("deviceId", gMacAddress);
//    firebasebJson.add("readVoltage", gReadVoltage);
    firebasebJson.set("uid", gCurrentDateTimeString);
    firebasebJson.set("temperature", gTemperature);
    firebasebJson.set("humidity", gHumidity);
    firebasebJson.set("deviceId", gMacAddress);
    firebasebJson.set("readVoltage", gReadVoltage);

//    if (Firebase.pushJSON(firebaseData, nodeString, firebasebJson)) {
    if (Firebase.set(firebaseData, nodeString, firebasebJson)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.print("PUSH NAME: ");
      Serial.println(firebaseData.pushName());
      Serial.println("ETag: " + firebaseData.ETag());
      Serial.println("------------------------------------");
      Serial.println();
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
    
// ================================================
    /*
    // Create read sensor json object
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
  
    JsonObject& sensorValues = root.createNestedObject(gCurrentDateTimeString);
    sensorValues["uid"] = gCurrentDateTimeString;
    sensorValues["temperature"] = gTemperature;
    sensorValues["humidity"] = gHumidity;
    sensorValues["deviceId"] = gMacAddress;

    

    // Create the new sensor values node
    Firebase.set(nodeString, sensorValues); 
    */
    Serial.print("uid=");
    Serial.println(gCurrentDateTimeString);
    Serial.print("deviceId=");
    Serial.println(gMacAddress);
    Serial.print(" humid: ");
    Serial.print(gHumidity);
    Serial.print(" % temperature: ");
    Serial.print(gTemperature);
    Serial.println(" Celsius");

    delay(1000);
    /*
    // handle error
    if (Firebase.failed()) {
        Serial.print("setting /node data failed:");
        Serial.println(Firebase.error());  
        
        delay(1000);
        
        // resend sensor value to firebase again
        Firebase.set(nodeString, sensorValues); 
        if (Firebase.failed()) {
          Serial.print("resend /node data failed again:");
          Serial.println(Firebase.error());  
          return false;
        } 
    }
    */
    
    Serial.println("saveDataToCloudDatabase() Finished");
    Serial.println("");
  } else {
    Serial.println("get the current date time string failured!!");
    return false;
  }
  return true;
}


bool createDeviceDataToCloudDatabase(void) {
  int c = 0;

//  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
//  gFirebaseHost = FIREBASE_HOST;
//  gFirebaseAuth = FIREBASE_AUTH;
//  Serial.println("Waiting for create a new device data into the cloud database.");
//  Serial.print("Firebase Host:");
//  Serial.println(gFirebaseHost);
//  Serial.print("Firebase Auth:");
//  Serial.println(gFirebaseAuth);

  // Get current date time uid.
  if(getCurrentDateTime()) {

    Serial.print("gCurrentDateTimeString=");
    Serial.println(gCurrentDateTimeString);

    // Create read sensor json object
//    StaticJsonBuffer<256> jsonBuffer;
//    JsonObject& root = jsonBuffer.createObject();
//  
//    JsonObject& deviceValues = root.createNestedObject(gMacAddress);
//    deviceValues["id"] = gMacAddress;
//    deviceValues["uid"] = gMacAddress;
//    deviceValues["name"] = gDeviceName;
//    deviceValues["localip"] = gDeviceLocalIp;

    // Append a new value to temporary node
//    String nodeString = "users/cray/devices/ht-00001;
    String nodeString = "users/cray/devices/" + gMacAddress;
    Serial.print("nodeString=");
    Serial.println(nodeString);
    
    Serial.print("uid=");
    Serial.println(gCurrentDateTimeString);
    Serial.print("deviceId=");
    Serial.println(gMacAddress);
    Serial.print("name: ");
    Serial.println(gDeviceName);
    Serial.print("localip: ");
    Serial.println(gDeviceLocalIp);

    // Create FirebaseJson object
//    firebasebJson.clear().add("uid", gCurrentDateTimeString);
//    firebasebJson.add("temperature", gTemperature);
//    firebasebJson.add("humidity", gHumidity);
//    firebasebJson.add("deviceId", gMacAddress);
//    firebasebJson.add("readVoltage", gReadVoltage);
    firebasebJson.set("id", gMacAddress);
    firebasebJson.set("uid", gMacAddress);
    firebasebJson.set("name", gDeviceName);
    firebasebJson.set("localip", gDeviceLocalIp);

    // Create the new sensor values node
    if (Firebase.set(firebaseData, nodeString, firebasebJson)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.print("PUSH NAME: ");
      Serial.println(firebaseData.pushName());
      Serial.println("ETag: " + firebaseData.ETag());
      Serial.println("------------------------------------");
      Serial.println();
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }

    /*
    // Create the new sensor values node
    Firebase.set(nodeString, deviceValues); 

    // handle error
    if (Firebase.failed()) {
        Serial.print("setting /node data failed:");
        Serial.println(Firebase.error());  
        return false;
    }
    */
    delay(1000);
    
    Serial.println("");
    Serial.println("createDeviceDataToCloudDatabase() Finished");

    // set working mode from setup mode to burst mode.
    if(updateWorkingMode(BURST_MODE)) {
      Serial.println("updateWorkingMode is successfully!! then reset esp to the new working burst mode->");
      ESP.reset();
    } else {
      Serial.println("updateWorkingMode failured!!");
    }
  } else {
    Serial.println("get the current date time string failured!!");
    return false;
  }
  return true;
}

bool updateWorkingMode(String qmode) {
  // save working mode into eeprom
  bool result = false;
//  String qmode = BURST_MODE;
  if (qmode.length() > 0) {
    // mode size 20 characters
    Serial.println("writing eeprom mode:");
    for (int i = 0; i < qmode.length(); ++i)
    {
      EEPROM.write(224 + i, qmode[i]);
      Serial.print("Wrote: ");
      Serial.println(qmode[i]);
    }

    EEPROM.commit();

    delay(100);
    // Verify mode value
    String emode = "";
    for (int i = 224; i < 244; ++i)
    {
      emode += char(EEPROM.read(i));
    }
    Serial.print("verify MODE: ");
    Serial.println(emode);
    gWorkingMode = emode;
    
    result = true;
  }
  return result;
}

//------------------------- Functions about sensor

/**
 * Get the current date time for uid of read sensor on the cloud database.
 */
bool getCurrentDateTime(void) {
  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +7 = 25200
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(BANGKOK_TIMEZONE);
  timeClient.update();

  //Get a time structure
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(monthDay);

  int currentMonth = ptm->tm_mon+1;
  Serial.print("Month: ");
  Serial.println(currentMonth);

  String currentMonthName = months[currentMonth-1];
  Serial.print("Month name: ");
  Serial.println(currentMonthName);

  int currentYear = ptm->tm_year+1900;
  Serial.print("Year: ");
  Serial.println(currentYear);

  //Print complete date:
//  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  String formattedTime = timeClient.getFormattedTime();
  
  char date_buf[20];
  sprintf(date_buf,"%04u-%02u-%02u %s", currentYear,currentMonth,monthDay, formattedTime.c_str());
  Serial.print("date_buf=");
  Serial.println(date_buf);

  gCurrentDateTimeString = date_buf;

  return true;
}

bool readSensor(void) {
  dht.begin();

  // Read temp & Humidity for DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(500);
    return false;
  }

  // Get sensor values
  gTemperature = t;
  gHumidity = h;
  
  return true;
}

//------------------------- Functions about reset button
bool listenResetButton(long elapsedMS) {
  bool result = false;
  gIsResetButtonPress = digitalRead(resetButton);
  Serial.print("gIsResetButtonPress=");
  Serial.println(gIsResetButtonPress);

  if(gIsResetButtonPress == HIGH) {
    if(gResetCount == 3) {
      blinkLED();
      // Verify reset button pressing
      Serial.println("Doing reset to manufacturing mode...");
      
      // Reset to manufacturing mode
      if(resetManufacturingMode()) {
        Serial.println("Reset to manufacturing mode sucessfully!!");
      }
      
      gResetCount = 0;
      result = true;
    } else {
      Serial.print("Reset button ON gResetCount=");
      Serial.println(gResetCount);                           
//      digitalWrite(2, HIGH);
      gResetCount++;
      result = true;
    }
  } else {
      Serial.print("Reset button OFF gResetCount=");
      Serial.println(gResetCount);
      
      delay(1000);
      // Not press reset button, then continue normal tasks process
      processNormalTasksEngine(elapsedMS);
  }
  return result;
}

void blinkLEDOnce(long elapsedMS) {
  static long ledTime = 0;  // static define value once only
  ledTime = ledTime + elapsedMS;
 
  if(ledTime >= led_interval) {
    ledState = !ledState;
    digitalWrite(D0_LED_BUILTIN,ledState);   
    ledTime = ledTime - led_interval;

    // listen local webserver every 1 seconds
    server.handleClient();
    MDNS.update();
  }
}

void blinkLED() {
  // Blink 3 times 
  for (int i = 0; i < 3; i++) {
    digitalWrite(D0_LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(D0_LED_BUILTIN, LOW);
    delay(100);
  }
}
//------------------------- Functions about reset to manufacturing mode
bool resetManufacturingMode(void){
  bool result = false;
  /***
    Iterate through each byte of the EEPROM storage.

    Larger AVR processors have larger EEPROM sizes, E.g:
    - Arduno Duemilanove: 512b EEPROM storage.
    - Arduino Uno:        1kb EEPROM storage.
    - Arduino Mega:       4kb EEPROM storage.

    Rather than hard-coding the length, you should use the pre-provided length function.
    This will make your code portable to all AVR processors.
  ***/
  EEPROM.begin(512);
  delay(800);
  Serial.println("Erasing...");

  if ( RESET_EEPROM ) {
    for (int i = 0; i < 512; i++) {
      EEPROM.write(i, 0);
      Serial.print(".");
    }
    EEPROM.commit();
    delay(500);

    EEPROM.end();

    blinkLED();
    
    Serial.println("");
    Serial.println("EEPROM erased!!");

    if(updateWorkingMode(SETUP_MODE)) {
      // Reset NodeMCU
      Serial.println("Reset NodeMCU!!**");
      ESP.reset();
    } else {
      Serial.println("EEPROM erased failured??!!");
    }
  }
  
  return true;
}

float readVoltage() { // read internal VCC
  float volts = ESP.getVcc();
  Serial.printf("The internal VCC reads %1.2f volts\n", volts / 1000);
  char voltage[5];
  sprintf(voltage,"%1.2f", volts / 1000);
  Serial.print("voltage=");
  Serial.println(voltage);
  
  gReadVoltage = volts / 1000;
  Serial.print("gReadVoltage=");
  Serial.println(gReadVoltage);
  return volts;
}

void initialFirebase(void) {
  Serial.println("Initial Firebase to ready to get/set cloud database value.");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData.setResponseSize(1024);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);

  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
  
  gFirebaseHost = FIREBASE_HOST;
  gFirebaseAuth = FIREBASE_AUTH;
  Serial.println("Waiting for create a new device data into the cloud database.");
  Serial.print("Firebase Host:");
  Serial.println(gFirebaseHost);
  Serial.print("Firebase Auth:");
  Serial.println(gFirebaseAuth);
}

String getDeviceReadingIntervalFromCloudDatabase() {
  String result = "10000";
  String nodeString = "users/cray/devices/" + gMacAddress + "/readingInterval";
  Serial.print("nodeString: ");
  Serial.println(nodeString);
  
  // get value 
  if(Firebase.getInt(firebaseData, nodeString)) {
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());  
//    FirebaseJson &json = firebaseData.jsonObject();
//    FirebaseJsonData data;
//    json.get(data, "/readingInterval");
    result = String(firebaseData.intData());
    Serial.print("readingInterval: ");
    Serial.println(result);

    if(result == "") {
      Serial.println("readingInterval not found, then set default value to 10000");
      result = "10000";
    }
  } else {
    Serial.println("errPATH: " + firebaseData.dataPath());
    Serial.println("errTYPE: " + firebaseData.dataType());  
  // handle error
//  if (Firebase.failed()) {
      Serial.print("getting readingInterval failed: ");
//      Serial.println(Firebase.error());  
//      Serial.println(Firebase.error());  
      Serial.println("Error : " + firebaseData.errorReason());
      delay(1000);
      return result;
  }
  
  return result;
}

bool writeLocalReadingIntervalConfig() {
  bool result = false;

  // (device)reading interval size 10 characters. (1 week = 168 hours = 604,800,000 milliseconds)
  // Clear part
  for (int i = 0; i < 10; ++i) { 
    EEPROM.write(308 + i, 0);
  }
  Serial.print("writing eeprom device reading interval:");
  Serial.println(gDeviceReadingInterval);
  for (int i = 0; i < gDeviceReadingInterval.length(); ++i) {
    EEPROM.write(308 + i, gDeviceReadingInterval[i]);
    Serial.print("Wrote: ");
    Serial.println(gDeviceReadingInterval[i]);
  }

  // submit eeprom saving 
  EEPROM.commit();

  result = true;
  return result;
}
