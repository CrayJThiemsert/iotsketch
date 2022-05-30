/**

   Written and Verified by 
   Mr. Jirachai Thiemsert
   Project: IOT System
   Created: FEB 2021
   Modified: OCT 2021

*/
// To connect Wifi
#include<FirebaseESP8266.h>
#include "Arduino.h"
#include <EMailSender.h>
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


// To get the current date time from internet
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

// ======================================================================
// Default "theNode" AP Mode SSID and password, use to transfer data between "the App" and "the Node".
#define THE_NODE_SSID "theNode_DHT"
#define THE_NODE_PASSWORD ""
#define BANGKOK_TIMEZONE 25200

const char* host = "thenode";
//IPAddress ap_local_ip = {192,168,1,199};   // Set up "the Node"'s AP mode IP
IPAddress ap_local_ip = {192,168,4,99};   // Set up "the Node"'s AP mode IP
IPAddress gateway={192,168,4,9};      // Set up "the Node"'s AP mode Gateway
IPAddress subnet={255,255,255,0};     // Set up "the Node"'s AP mode Subnet
// ======================================================================
// NTP Servers:
static const char ntpServerName[] = "us.pool.ntp.org";
// Bangkok timezone GMT+7
const int timeZone = 7;  // Bangkok Time Zone


const int IOT_NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
//const long BANGKOK_TIMEZONE = 25200;
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
unsigned int localPort = 8888;  // local port to listen for UDP packets

//NTPClient timeClient(ntpUDP);
//NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, BANGKOK_TIMEZONE);

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};


// ======================================================================
// Config DHT - Humid and Temperature Sensor
// #define DHTPIN 4 // PIN 4 = D2 = GPIO4
#define DHTPIN 12 // PIN 12 = D6 = GPIO12
//#define DHTTYPE DHT11
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
// ======================================================================
#define FIREBASE_HOST "asset-management-lff.firebaseio.com"
#define FIREBASE_AUTH "tyPYNCdjHQREFNsW46sha4JhOFG4WG4U7pL8iShx"
#define DEFAULT_READING_INTERVAL "10000"
#define DEFAULT_READING_INTERVAL_LONG 10000

#define DEFAULT_NOTIFY_HUMID_LOWER 0
#define DEFAULT_NOTIFY_HUMID_HIGHER 999
#define DEFAULT_NOTIFY_TEMP_LOWER 0
#define DEFAULT_NOTIFY_TEMP_HIGHER 999
#define DEFAULT_NOTIFY_EMAIL "cray.j.thiemsert@gmail.com"
#define DEFAULT_NOTIFY_IS_SEND_NOTIFY false
// ======================================================================
// Set to true to reset eeprom before to write something
#define RESET_EEPROM true
#define D0_LED_BUILTIN 2
// ======================================================================
String DEFAULT_FBDB_USER = "cray";
// ======================================================================
// Variables
// ======================================================================
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
String gUserUid = "";

float gNotifyHumidLower = 0;
float gNotifyHumidHigher = 999;
float gNotifyTempLower = 0;
float gNotifyTempHigher = 999;
String gNotifyEmail = "";
bool gIsSendNotify = false;

// Function Decalration
bool testWifi(void);
void createWebServer(void);
void launchWeb(void);
void setupAP(void);
bool getCurrentDateTimeOldNotStable(void);
bool getCurrentDateTime(void);
time_t getNtpTime();
void printDigits(int digits);

bool readSensor(void);
bool saveDataToCloudDatabase(void);
bool createDeviceDataToCloudDatabase(void);
bool updateWorkingMode(String qmode);
bool listenResetButton(long elapsedMS);
bool resetManufacturingMode(void);
void processNormalTasksEngine(unsigned long currentMillis);
bool loadInternalConfig(void);
bool processReadSensor(void);
bool processNotification(void);
bool processSendNotificationEmail(String notifyType);
float readVoltage(void); // read internal VCC
String getDeviceReadingIntervalFromCloudDatabase(void);
bool getNotificationInfoFromCloudDatabase(void);
String getDeviceValueFromCloudDatabase(String key);
bool writeLocalNotificationInfo(void);

bool writeLocalReadingIntervalConfig(void);
bool writeLocalNotificationConfig(int memPos, int memSize, String value);

void initialFirebase(void);
void processDeepSleep(void);
// Buttons
const int resetButton = 16;
int gIsResetButtonPress = 0;
int gResetCount = 0;
bool writeLocalUserUidConfig(void);

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

ADC_MODE(ADC_VCC);  // allows you to monitor the internal VCC level; it varies with WiFi load

// Email settings
EMailSender emailSend("cray.j.tester@gmail.com", "ntptwuzbtnrjneuo");

void setup() {

  Serial.begin(115200); //Initialising if(DEBUG)Serial Monitor
  // Wait for serial to initialize.
  while(!Serial) { }
  
  Serial.println();
  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  delay(250);
  
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

  // Disable the WiFi persistence.  The ESP8266 will not load and save WiFi settings in the flash memory.
  WiFi.persistent( false );

  // Try to connect wifi by loaded ssid and password values from eeprom
////  WiFi.begin(esid.c_str(), epass.c_str());
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(gSSID, gSSIDPassword);
  if (testWifi())  {
    while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
      Serial.print ( "." );
    }

    // Initialize a NTPClient to get time
    Serial.println("Starting UDP");
    ntpUDP.begin(localPort);
    Serial.print("Local port: ");
    Serial.println(ntpUDP.localPort());
    Serial.println("waiting for sync");
    setSyncProvider(getNtpTime);
    setSyncInterval(300);
    
//    timeClient.begin();
//    // Set offset time in seconds to adjust for your timezone, for example:
//    // GMT +1 = 3600
//    // GMT +7 = 25200
//    // GMT +8 = 28800
//    // GMT -1 = -3600
//    // GMT 0 = 0
//    timeClient.setTimeOffset(BANGKOK_TIMEZONE);
//
//    getCurrentDateTime();

    Serial.print("0: gCurrentDateTimeString=");
    Serial.println(gCurrentDateTimeString);
    
    Serial.println("Succesfully Internet Wifi Connected!!!");

    

    // Initial Firebase to ready to get/set cloud database value
    initialFirebase();

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

    


    

//    Serial.println("=>Call Device's notifyEmail information from cloud database: getDeviceValueFromCloudDatabase()");
//    getDeviceValueFromCloudDatabase("notifyEmail");

    // Test read and update readingInterval value from cloud database
    /*
    // launch local webserver to wait for setup request from "theApp".
    launchWeb();
    
    Serial.println();
    Serial.println("Waiting update settings device trigger.");
    */

    
//    while ((WiFi.status() != WL_CONNECTED))  {
//      Serial.println(".");
//      delay(1000); // ** here
//      server.handleClient();
//      MDNS.update();
//    }

    if(writeLocalNotificationInfo()) {
      Serial.println("Write update device info into EEPROM is successfully!!");
    } else {
      Serial.println("Write update device info into EEPROM is failed!!");
    }
    
    processDeepSleep();
    return; // never get in this line
  } else  {
    Serial.println("Turning the HotSpot On");
    setupAP();// Setup HotSpot

    Serial.println();
    Serial.println("Waiting add new device trigger.");
    
    while ((WiFi.status() != WL_CONNECTED))  {
      Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
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

void loop() {
}

void processDeepSleep(void) {
  Serial.print("gDeviceReadingInterval=");
  Serial.println(gDeviceReadingInterval);
  long readingInterval = strtol(gDeviceReadingInterval.c_str(), NULL, 0 );
   if(readingInterval == 0) {
    readingInterval = DEFAULT_READING_INTERVAL_LONG;
   }
//  ESP.deepSleep(10E6, WAKE_RFCAL); // good night!  D0 fires a reset in 10 seconds...
  unsigned long sleepInterval = readingInterval * 1000;
  Serial.print("I am going to sleep and will wakeup in next ");
  Serial.print(sleepInterval);
  Serial.println(" microsecond");

   // Ensure the chip goes into proper deepsleep. Without them, the chip usually ends up consuming about 1.2 mA of current while sleeping.
  WiFi.disconnect( true );
  delay( 1 );
  
  ESP.deepSleep(sleepInterval, WAKE_RFCAL); // good night!  D0 fires a reset in readingInterval seconds...(or default 10 second)
//  ESP.deepSleep(sleepInterval, WAKE_NO_RFCAL); // good night!  D0 fires a reset in 10 seconds...
//  ESP.deepSleep(sleepInterval); // good night!  D0 fires a reset in 10 seconds...
}

void processNormalTasksEngine(unsigned long currentMillis) {
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

bool loadInternalConfig(void) {
  bool result = false;

  EEPROM.begin(512); //Initialasing EEPROM
  delay(100);
  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println(".");
  Serial.println("sketch_2022mar08a_iot_interval_fixed_date_1970_bug_optimize_loop");
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

  // -------- Notification Settings --------------
  // -------- notifyHumidLower --------------
  String enotifyhumidlower = "";
  for (int i = 318; i < 328; ++i)  { // size 10
    enotifyhumidlower += char(EEPROM.read(i));
  }
  Serial.print("READING DEFAULT_NOTIFY_HUMID_LOWER: ");
  Serial.println(enotifyhumidlower);
  enotifyhumidlower.trim();
  
  if(enotifyhumidlower.equals("")) {
    gNotifyHumidLower = DEFAULT_NOTIFY_HUMID_LOWER;
    Serial.print("empty DEFAULT_NOTIFY_HUMID_LOWER then use default: ");
    Serial.println(gNotifyHumidLower);
  } else {
    gNotifyHumidLower = enotifyhumidlower.toFloat();  
  }

  // -------- notifyHumidHigher --------------
  String enotifyhumidhigher = "";
  for (int i = 328; i < 338; ++i)  { // size 10
    enotifyhumidhigher += char(EEPROM.read(i));
  }
  Serial.print("READING DEFAULT_NOTIFY_HUMID_HIGHER: ");
  Serial.println(enotifyhumidhigher);
  enotifyhumidhigher.trim();
  
  if(enotifyhumidhigher.equals("")) {
    gNotifyHumidHigher = DEFAULT_NOTIFY_HUMID_HIGHER;
    Serial.print("empty DEFAULT_NOTIFY_HUMID_HIGHER then use default: ");
    Serial.println(gNotifyHumidHigher);
  } else {
    gNotifyHumidHigher = enotifyhumidhigher.toFloat();  
  }

  // -------- notifyTempLower --------------
  String enotifytemplower = "";
  for (int i = 338; i < 348; ++i)  { // size 10
    enotifytemplower += char(EEPROM.read(i));
  }
  Serial.print("READING DEFAULT_NOTIFY_TEMP_LOWER: ");
  Serial.println(enotifytemplower);
  enotifytemplower.trim();
  
  if(enotifytemplower.equals("")) {
    gNotifyTempLower = DEFAULT_NOTIFY_TEMP_LOWER;
    Serial.print("empty DEFAULT_NOTIFY_TEMP_LOWER then use default: ");
    Serial.println(gNotifyTempLower);
  } else {
    gNotifyTempLower = enotifytemplower.toFloat();
  }

  // -------- notifyTempHigher --------------
  String enotifytemphigher = "";
  for (int i = 348; i < 358; ++i)  { // size 10
    enotifytemphigher += char(EEPROM.read(i));
  }
  Serial.print("READING DEFAULT_NOTIFY_TEMP_HIGHER: ");
  Serial.println(enotifytemphigher);
  enotifytemphigher.trim();
  
  if(enotifytemphigher.equals("")) {
    gNotifyTempHigher = DEFAULT_NOTIFY_TEMP_HIGHER;
    Serial.print("empty DEFAULT_NOTIFY_TEMP_HIGHER then use default: ");
    Serial.println(gNotifyTempHigher);
  } else {
    gNotifyTempHigher = enotifytemphigher.toFloat();
  }

  // -------- notifyEmail --------------
  String enotifyemail = "";
  for (int i = 358; i < 422; ++i)  { // size 64
    enotifyemail += char(EEPROM.read(i));
  }
  Serial.print("READING DEFAULT_NOTIFY_EMAIL: ");
  Serial.println(enotifyemail);
  enotifyemail.trim();
  gNotifyEmail = enotifyemail;
  if(gNotifyEmail.equals("")) {
    gNotifyEmail = DEFAULT_NOTIFY_EMAIL;
    Serial.print("empty DEFAULT_NOTIFY_EMAIL then use default: ");
    Serial.println(gNotifyEmail);
  }

  // -------- isSendNotify --------------
  String eissendnotify = "";
  for (int i = 422; i < 423; ++i)  { // size 1
    eissendnotify += char(EEPROM.read(i));
  }
  Serial.print("READING DEFAULT_NOTIFY_EMAIL: ");
  Serial.println(eissendnotify);
  eissendnotify.trim();
  
  if(eissendnotify.equals("")) {
    gIsSendNotify = DEFAULT_NOTIFY_IS_SEND_NOTIFY;
    Serial.print("empty DEFAULT_NOTIFY_IS_SEND_NOTIFY then use default: ");
    Serial.println(gIsSendNotify);
  } else {
    gIsSendNotify = !(eissendnotify == "0");
  }
  
  return result;
}

bool processSendNotificationEmail(String notifyType) {
  bool result = false;
  

  EMailSender::EMailMessage message;
  Serial.println("EMailSender::EMailMessage message;");
  
  String messageSubject = "-";
  String messageBody = "-";
  Serial.println("declared default subject and body");
  if(notifyType.equals("humidity")) {
    // humidity
    messageSubject = gDeviceName + " - [Humidity Notification]";

    if(gHumidity < gNotifyHumidLower) {
      messageBody = "Greetings,<br><br>Humidity " + String(gHumidity) + " is lower than " + String(gNotifyHumidLower) + ".<br><br>Regards,<br>IoT System";  
    } else {
      if(gHumidity > gNotifyHumidHigher) {
        messageBody = "Greetings,<br><br>Humidity " + String(gHumidity) + " is higher than " + String(gNotifyHumidHigher) + ".<br><br>Regards,<br>IoT System";  
      }
    }
    
  } else {
    if(notifyType.equals("temperature")) {
      // temperature
      messageSubject = gDeviceName + " - [Temperature Notification]";
  
      if(gTemperature < gNotifyTempLower) {
        messageBody = "Greetings,<br><br>Temperature " + String(gTemperature) + " is lower than " + String(gNotifyTempLower) + ".<br><br>Regards,<br>IoT System";  
      } else {
        if(gTemperature > gNotifyTempHigher) {
          messageBody = "Greetings,<br><br>Temperature " + String(gTemperature) + " is higher than " + String(gNotifyTempHigher) + ".<br><br>Regards,<br>IoT System";  
        }
      }
    }
  }

  message.subject = messageSubject;
  message.message = messageBody;
//  message.subject = "Soggetto";
//  message.message = "Ciao come stai<br>io bene.<br>www.mischianti.org";

  Serial.println("assign default subject and body");
 
//    EMailSender::Response resp = emailSend.send("account_to_send@gmail.com", message);
  EMailSender::Response resp = emailSend.send(gNotifyEmail, message);
//  EMailSender::Response resp = emailSend.send("cray.j.thiemsert@gmail.com", message);
  Serial.println("sent email to [" + gNotifyEmail + "]");

  Serial.println("Sending status: ");

  Serial.println(resp.status);
  Serial.println(resp.code);
  Serial.println(resp.desc);

  Serial.print("(resp.status == 1)=");
  Serial.println(resp.status == 1);
  Serial.print("(resp.code == 0)=");
  Serial.println(resp.code == 0);
  Serial.print("(resp.desc.equals(Message sent!))=");
  Serial.println(resp.desc.equals("Message sent!"));
  if(resp.desc.equals("Message sent!")) {
    Serial.print(">>>Yes, it sent.");
    
    result = true;
  } else {
    Serial.print(">>>No, it did not send.");
  }
  Serial.print("Sending result: ");
    Serial.println(result);
  return result;
}

bool processNotification(void) {
  bool result = false;

  if(gHumidity < gNotifyHumidLower || gHumidity > gNotifyHumidHigher) {
    Serial.println("Humidity is over monitoring range. Sending notification...");
    result = processSendNotificationEmail("humidity");
  } else {
    Serial.println("Humidity is under monitoring range. No need to send notification.");
  }

  if(gTemperature < gNotifyTempLower || gTemperature > gNotifyTempHigher) {
    Serial.println("Temperature is over monitoring range. Sending notification...");
    Serial.println("Temperature " + String(gTemperature) + " is lower than " + String(gNotifyTempLower) + ".<br>Regards,<br>IoT System");
    result = processSendNotificationEmail("temperature");
  } else {
    Serial.println("Temperature is under monitoring range. No need to send notification.");
  }

  return result;
}

bool processReadSensor(void) {
  bool result = false;

  Serial.println("doing read sensor...");
  if(readSensor()) {
    Serial.println("Succesfully read sensor data!!!++++++++");

    // ==========================================
//    EMailSender::EMailMessage message;
//    message.subject = "Soggetto";
//    message.message = "Ciao come stai<br>io bene.<br>www.mischianti.org";
// 
////    EMailSender::Response resp = emailSend.send("account_to_send@gmail.com", message);
//    EMailSender::Response resp = emailSend.send("cray.j.thiemsert@gmail.com", message);
// 
//    Serial.println("Sending status: ");
// 
//    Serial.println(resp.status);
//    Serial.println(resp.code);
//    Serial.println(resp.desc);
    // ==========================================
    Serial.print('gIsSendNotify=');
    Serial.println(gIsSendNotify);
    if(gIsSendNotify) {
      Serial.println("Notification verifying...");
      if(processNotification()) {
        Serial.println("Notification process is successfully!!!");
        Serial.println("");
        result = true;
      } else {
        Serial.println("Sent Notification Failured!!!");
      }
    } else {
      Serial.println("Skip notification process.");
    }

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
    } else {
      Serial.println("readingInterval value equals then do nothing");
    }

    // Call Device information from cloud database
    Serial.println("=>Call Notification information from cloud database: getNotificationInfoFromCloudDatabase()");
    getNotificationInfoFromCloudDatabase();
    
    
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

void launchWeb(void) {
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

//  WiFi.softAP(THE_NODE_SSID, THE_NODE_PASSWORD); // Set Soft SSID
  boolean result = WiFi.softAP(THE_NODE_SSID, THE_NODE_PASSWORD); // Set Soft SSID
  Serial.print("Set softap...");
  if(result == true) {
    Serial.println("Ready");
  } else {
    Serial.println("Failed!");
  }

//  result = WiFi.softAPConfig(ap_local_ip,gateway,subnet); // Set up to module  
//  Serial.print("Set softAPConfig...");
//  if(result == true) {
//    Serial.println("Ready");
//  } else {
//    Serial.println("Failed!");
//  }
  
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
      String quseruid = server.arg("useruid");

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
//        for (int i = 0; i < 318; ++i) { // + reading interval
        for (int i = 0; i < 398; ++i) { // + user uid 80 characters
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

      // save user uid into eeprom
      if (quseruid.length() > 0) {
        hasValues = true;
        // user uid size 80 characters.
        // Clear part
        for (int i = 0; i < 80; ++i) { 
          EEPROM.write(318 + i, 0);
        }
        Serial.println("writing eeprom user uid:");
        for (int i = 0; i < quseruid.length(); ++i) {
          EEPROM.write(318 + i, quseruid[i]);
          Serial.print("Wrote: ");
          Serial.println(quseruid[i]);
        }
      }
      
//      // Test print out interval argument
//      if (qinterval.length() > 0) {
//        hasValues = true;        
//        wantReset = true;
//        Serial.print("qinterval=");
//        for (int i = 0; i < qinterval.length(); ++i) {
//          EEPROM.write(308 + i, qinterval[i]);
//          Serial.print("Wrote: ");
//          Serial.println(qinterval[i]);
//        }
//      }
      
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
  FirebaseJson firebasebJson;
  
  readVoltage();  // read internal VCC

  // Get current date time uid.
  if(getCurrentDateTime()) {

    Serial.print("gCurrentDateTimeString=");
    Serial.println(gCurrentDateTimeString);
    // Append a new value to temporary node
//    String nodeString = "users/cray/devices/ht-00001/ht-00001_history/" + gCurrentDateTimeString;
//    String nodeString = "users/cray/devices/" + gMacAddress + "/" + gMacAddress + "_history/" + gCurrentDateTimeString;
    String nodeString = "users/" + DEFAULT_FBDB_USER + "/devices/" + gMacAddress + "/" + gMacAddress + "_history/" + gCurrentDateTimeString;
    Serial.print("nodeString=");
    Serial.println(nodeString);

    // Create FirebaseJson object
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

      // Save latest sensor read values and battery voltage to device root node
      saveDeviceRootDataToCloudDatabase();
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
    
// ================================================
    Serial.print("uid=");
    Serial.println(gCurrentDateTimeString);
    Serial.print("deviceId=");
    Serial.println(gMacAddress);
    Serial.print(" humid: ");
    Serial.print(gHumidity);
    Serial.print(" % temperature: ");
    Serial.print(gTemperature);
    Serial.println(" Celsius");

    delay(600);
    
    Serial.println("saveDataToCloudDatabase() Finished");
    Serial.println("");
  } else {
    Serial.println("get the current date time string failured!!");
    return false;
  }
  return true;
}

bool saveDeviceRootDataToCloudDatabase(void) {
  bool result = false;
//   Get current date time uid.
//  if(getCurrentDateTime()) {
    FirebaseJson firebasebJson;
    Serial.println("");
    Serial.println("===saveDeviceRootDataToCloudDatabase===============");
    Serial.print("gCurrentDateTimeString=");
    Serial.println(gCurrentDateTimeString);
  
    gDeviceLocalIp = IpAddress2String(WiFi.localIP());
    Serial.print("gDeviceLocalIp: ");
    Serial.println(gDeviceLocalIp);
    Serial.print("gDeviceReadingInterval: ");
    Serial.println(gDeviceReadingInterval);
    
    // Append a new value to temporary node
  //    String nodeString = "users/cray/devices/ht-00001";
//    String nodeString = "users/cray/devices/" + gMacAddress;
    String nodeString = "users/" + DEFAULT_FBDB_USER + "/devices/" + gMacAddress;
    
    Serial.print("nodeString=");
    Serial.println(nodeString);
    
  
    // Create FirebaseJson object
    firebasebJson.set("updatedWhen", gCurrentDateTimeString);
    firebasebJson.set("temperature", gTemperature);
    firebasebJson.set("humidity", gHumidity);
    firebasebJson.set("readVoltage", gReadVoltage);
  
    firebasebJson.set("id", gMacAddress);
    firebasebJson.set("uid", gMacAddress);
    firebasebJson.set("name", gDeviceName);
    firebasebJson.set("localip", gDeviceLocalIp);
    firebasebJson.set("readingInterval", gDeviceReadingInterval.toFloat());

    Serial.print(" humid: ");
    Serial.print(gHumidity);
    Serial.print(" % temperature: ");
    Serial.print(gTemperature);
    Serial.println(" Celsius");

    if (Firebase.updateNode(firebaseData, nodeString, firebasebJson)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData.dataPath());
      Serial.print("PUSH NAME: ");
      Serial.println(firebaseData.pushName());
      Serial.println("ETag: " + firebaseData.ETag());
      Serial.println("------------------------------------");
      Serial.println();
      result = true;
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
    
  // ================================================
      delay(600);
    
    Serial.println("saveDeviceRootDataToCloudDatabase() Finished");
    Serial.println("");
    
    return result;
//  } else {
//    Serial.println("get the current date time string failured!!");
//    Serial.print("gCurrentDateTimeString=");
//    Serial.println(gCurrentDateTimeString);
//    Serial.println("saveDeviceRootDataToCloudDatabase() failured!!");
//    Serial.println("");
//    return false;
//  }
}

bool createDeviceDataToCloudDatabase(void) {
  int c = 0;
  FirebaseJson firebasebJson;

  // Get current date time uid.
  if(getCurrentDateTime()) {

    Serial.print("gCurrentDateTimeString=");
    Serial.println(gCurrentDateTimeString);

    // Create read sensor json object
    // Append a new value to temporary node
//    String nodeString = "users/cray/devices/ht-00001;
//    String nodeString = "users/cray/devices/" + gMacAddress;
    String nodeString = "users/" + DEFAULT_FBDB_USER + "/devices/" + gMacAddress;
    
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

  delay(300);
  char date_buf[20];
  sprintf(date_buf,"%04u-%02u-%02u %02u:%02u:%02u", year(),month(),day(), hour(), minute(), second());
  Serial.print("date_buf=");
  Serial.println(date_buf);
  gCurrentDateTimeString = date_buf;

  if(year() < 1971) {
    // recall it again until get the right date.
    Serial.println("recall it again until get the right date...");
    getCurrentDateTime();
  } else {
    return true;  
  }

  
}
 
bool getCurrentDateTimeOldNotStable(void) {
//  // Initialize a NTPClient to get time
//  timeClient.begin();
//  // Set offset time in seconds to adjust for your timezone, for example:
//  // GMT +1 = 3600
//  // GMT +7 = 25200
//  // GMT +8 = 28800
//  // GMT -1 = -3600
//  // GMT 0 = 0
//  timeClient.setTimeOffset(BANGKOK_TIMEZONE);
  timeClient.update();

  delay(500);

  //Get a time structure
  unsigned long epochTime = timeClient.getEpochTime();
//  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  struct tm *ptm = localtime ((time_t *)&epochTime); 
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);

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
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);  
  
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

void blinkLED(void) {
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

float readVoltage(void) { // read internal VCC
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

  Serial.print("Firebase Host:");
  Serial.println(gFirebaseHost);
  Serial.print("Firebase Auth:");
  Serial.println(gFirebaseAuth);
}

String getDeviceValueFromCloudDatabase(String key) {
  
  
  String result = "0";
//  String nodeString = "users/cray/devices/" + gMacAddress + "/readingInterval";
//  String nodeString = "users/" + DEFAULT_FBDB_USER + "/devices/" + gMacAddress + "/" + key;
  String nodeString = "users/" + DEFAULT_FBDB_USER + "/devices/" + gMacAddress + "/notification/" + key;
  
  Serial.print("nodeString: ");
  Serial.println(nodeString);
  
  // get value 
  if(Firebase.get(firebaseData, nodeString)) {
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());  
    if (firebaseData.dataType() == "int") {
      Serial.println(firebaseData.intData());
      result = String(firebaseData.intData());
    } else if(firebaseData.dataType() == "string"){
      result = firebaseData.stringData();
      result.trim();
    }
    
    Serial.print(key + ": ");
    Serial.println(result);

    if(result == "") {
      Serial.println(key + " not found!!");
      result = "0";
    }
  } else {
    Serial.println("errPATH: " + firebaseData.dataPath());
    Serial.println("errTYPE: " + firebaseData.dataType());  
  // handle error
//  if (Firebase.failed()) {
      Serial.print("getting " + key + " failed: ");
//      Serial.println(Firebase.error());  
//      Serial.println(Firebase.error());  
      Serial.println("Error : " + firebaseData.errorReason());
      delay(1000);
      return result;
  }
  
  return result;
}

bool writeLocalNotificationInfo(void) {
  bool result = false;

  if(!writeLocalNotificationConfig(318, 10, String(gNotifyHumidLower))) {
    Serial.println("writeLocalNotificationConfig [gNotifyHumidLower] is failured!!!");
    return result;
  }

  if(!writeLocalNotificationConfig(328, 10, String(gNotifyHumidHigher))) {
    Serial.println("writeLocalNotificationConfig [gNotifyHumidHigher] is failured!!!");
    return result;
  }

  if(!writeLocalNotificationConfig(338, 10, String(gNotifyTempLower))) {
    Serial.println("writeLocalNotificationConfig [gNotifyTempLower] is failured!!!");
    return result;
  }

  if(!writeLocalNotificationConfig(348, 10, String(gNotifyTempHigher))) {
    Serial.println("writeLocalNotificationConfig [gNotifyTempHigher] is failured!!!");
    return result;
  }

  if(!writeLocalNotificationConfig(358, 64, gNotifyEmail)) {
    Serial.println("writeLocalNotificationConfig [gNotifyEmail] is failured!!!");
    return result;
  }

  if(!writeLocalNotificationConfig(422, 1, String(gIsSendNotify))) {
    Serial.println("writeLocalNotificationConfig [gIsSendNotify] is failured!!!");
    return result;
  }

  if(!writeLocalReadingIntervalConfig()) {
    Serial.println("writeLocalReadingIntervalConfig is failured!!!");
    return result;
  }
  
  
  result = true;
  return result;
}

bool getNotificationInfoFromCloudDatabase(void) {
  // Prepare FirebaseJson to take the JSON object from FirebaseJsonArray
//  FirebaseJsonData result;
//  FirebaseJson json;
  String nodeString = "users/" + DEFAULT_FBDB_USER + "/devices/" + gMacAddress + "/notification";

  Serial.print("nodeString:");
  Serial.println(nodeString);

//  if(Firebase.getString(firebaseData, nodeString)) {
//  if(Firebase.get(firebaseData, nodeString)) {
  QueryFilter query;
//  query.orderBy("$key").limitToFirst(12);
  query.orderBy("$key");
  
//  if(Firebase.getJSON(firebaseData, nodeString, query, &json)) {
  if(Firebase.getJSON(firebaseData, nodeString, query)) {
    //Success, then try to read the JSON payload value
//    Serial.println(firebaseData.jsonData());
  
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());

    Serial.println("jsonString: " + firebaseData.jsonString());
    Serial.println("stringData: " + firebaseData.stringData());

    FirebaseJson &json = firebaseData.jsonObject();
    FirebaseJsonData data;

    json.get(data, "/notifyHumidLower");
    float cNotifyHumidLower = data.floatValue;

    json.get(data, "/notifyHumidHigher");
    float cNotifyHumidHigher = data.floatValue;

    json.get(data, "/notifyTempLower");
    float cNotifyTempLower = data.floatValue;

    json.get(data, "/notifyTempHigher");
    float cNotifyTempHigher = data.floatValue;
    
    json.get(data, "/notifyEmail");
    String cNotifyEmail = data.stringValue;
    cNotifyEmail.trim();

    json.get(data, "/isSendNotify");
    bool cIsSendNotify = data.boolValue;
    
    

    // Do something
    Serial.print("cNotifyHumidLower:");
    Serial.println(cNotifyHumidLower);
    
    
    if(cNotifyHumidHigher == 0.0) {
      Serial.println("cNotifyHumidHigher is equal 0, then change it to DEFAULT_NOTIFY_HUMID_HIGHER[999].");
      cNotifyHumidHigher = DEFAULT_NOTIFY_HUMID_HIGHER;
    } 
    Serial.print("cNotifyHumidHigher:");
    Serial.println(cNotifyHumidHigher);
    
    Serial.print("cNotifyTempLower:");
    Serial.println(cNotifyTempLower);
    
    
    if(cNotifyTempHigher == 0.0) {
      Serial.println("cNotifyTempHigher is equal 0, then change it to DEFAULT_NOTIFY_TEMP_HIGHER[999].");
      cNotifyTempHigher = DEFAULT_NOTIFY_TEMP_HIGHER;
    } 
    Serial.print("cNotifyTempHigher:");
    Serial.println(cNotifyTempHigher);
    
    Serial.print("cNotifyEmail:");
    Serial.println(cNotifyEmail);
    

    
    // notifyHumidLower
    if(cNotifyHumidLower != gNotifyHumidLower) {
      Serial.println("");
      Serial.println("[gNotifyHumidLower] value has updated from cloud database, then we need to update value in eeprom, too.");
      gNotifyHumidLower = cNotifyHumidLower;

      
    } else {
      Serial.println("gNotifyHumidLower value equals then do nothing");
    }

    // notifyHumidHigher
    if(cNotifyHumidHigher != gNotifyHumidHigher) {
      Serial.println("");
      Serial.println("[gNotifyHumidHigher] value has updated from cloud database, then we need to update value in eeprom, too.");
      gNotifyHumidHigher = cNotifyHumidHigher;

      
    } else {
      Serial.println("gNotifyHumidHigher value equals then do nothing");
    }

    // notifyTempLower
    if(cNotifyTempLower != gNotifyTempLower) {
      Serial.println("");
      Serial.println("[gNotifyTempLower] value has updated from cloud database, then we need to update value in eeprom, too.");
      gNotifyTempLower = cNotifyTempLower;

      
    } else {
      Serial.println("gNotifyTempLower value equals then do nothing");
    }

    // notifyTempHigher
    if(cNotifyTempHigher != gNotifyTempHigher) {
      Serial.println("");
      Serial.println("[gNotifyTempHigher] value has updated from cloud database, then we need to update value in eeprom, too.");
      gNotifyTempHigher = cNotifyTempHigher;

      
    } else {
      Serial.println("gNotifyTempHigher value equals then do nothing");
    }

    // notifyEmail
    if(!cNotifyEmail.equals(gNotifyEmail)) {
      Serial.println("");
      Serial.println("[gNotifyEmail] value has updated from cloud database, then we need to update value in eeprom, too.");
      gNotifyEmail = cNotifyEmail;

      
    } else {
      Serial.println("gNotifyEmail value equals then do nothing");
    }

    // isSendNotify
    if(cIsSendNotify != gIsSendNotify) {
      Serial.println("");
      Serial.println("[gIsSendNotify] value has updated from cloud database, then we need to update value in eeprom, too.");
      gIsSendNotify = cIsSendNotify;

      
    } else {
      Serial.println("gIsSendNotify value equals then do nothing");
    }

    return true;
  } else {
      Serial.println("Error : " + firebaseData.errorReason());
      return false;
  }

  
}

String getDeviceReadingIntervalFromCloudDatabase(void) {
  
  
  String result = "10000";
//  String nodeString = "users/cray/devices/" + gMacAddress + "/readingInterval";
  String nodeString = "users/" + DEFAULT_FBDB_USER + "/devices/" + gMacAddress + "/readingInterval";
  
  Serial.print("nodeString: ");
  Serial.println(nodeString);
  
  // get value 
  if(Firebase.get(firebaseData, nodeString)) {
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());  
    if (firebaseData.dataType() == "int") {
      Serial.println(firebaseData.intData());
      result = String(firebaseData.intData());
    } else if(firebaseData.dataType() == "string"){
      result = firebaseData.stringData();
      result.trim();
    }
    
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

bool writeLocalReadingIntervalConfig(void) {
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

/* 
 * Write String value into EEPORM 
 */
bool writeLocalNotificationConfig(int memPos, int memSize, String value) {
  bool result = false;

  // (device)reading interval size 10 characters. (1 week = 168 hours = 604,800,000 milliseconds)
  // Clear part
  for (int i = 0; i < memSize; ++i) { 
    EEPROM.write(memPos + i, 0);
  }
  Serial.print("writing to eeprom :");
  Serial.println(value);
  for (int i = 0; i < value.length(); ++i) {
    EEPROM.write(memPos + i, value[i]);
    Serial.print("Wrote: ");
    Serial.println(value[i]);
  }

  // submit eeprom saving 
  EEPROM.commit();

  result = true;
  return result;
}

bool writeLocalUserUidConfig(void) {
  bool result = false;

  // user uid size 80 characters. 
  // Clear part
  for (int i = 0; i < 80; ++i) { 
    EEPROM.write(318 + i, 0);
  }
  Serial.print("writing eeprom user uid:");
  Serial.println(gUserUid);
  for (int i = 0; i < gUserUid.length(); ++i) {
    EEPROM.write(318 + i, gUserUid[i]);
    Serial.print("Wrote: ");
    Serial.println(gUserUid[i]);
  }

  // submit eeprom saving 
  EEPROM.commit();

  result = true;
  return result;
}

void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*-------- NTP code ----------*/


byte packetBuffer[IOT_NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (ntpUDP.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = ntpUDP.parsePacket();
    if (size >= IOT_NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      ntpUDP.read(packetBuffer, IOT_NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, IOT_NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  ntpUDP.beginPacket(address, 123); //NTP requests are to port 123
  ntpUDP.write(packetBuffer, IOT_NTP_PACKET_SIZE);
  ntpUDP.endPacket();
}
