#include <ESP8266WiFi.h>

#include<FirebaseArduino.h>
#include <DHT.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

//#include "RTClib.h"
//
//RTC_DS1307 RTC;

//#include <DateTime.h>
//#include <DateTimeStrings.h>
//
//#define TIME_MSG_LEN  11   // time sync to PC is HEADER and unix time_t as ten ascii digits
//#define TIME_HEADER  255   // Header tag for serial time sync message

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

// Bangkok timezone GMT+7
#define BANGKOK_TIMEZONE 25200


// Config Firebase
#define FIREBASE_HOST "asset-management-lff.firebaseio.com"
#define FIREBASE_AUTH "tyPYNCdjHQREFNsW46sha4JhOFG4WG4U7pL8iShx"

// Config connect WiFi
#define WIFI_SSID "Killing Me Softly 2G"
#define WIFI_PASSWORD "14mP455w0rd"

// Config DHT
#define DHTPIN 4
//#define DHTTYPE DHT11
#define DHTTYPE DHT22

String name;
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);

//  RTC.begin();
//
//  if (! RTC.isrunning()) {
//    Serial.println("RTC is NOT running!");
//    // following line sets the RTC to the date & time this sketch was compiled
//    RTC.adjust(DateTime(__DATE__, __TIME__));
//  }

  WiFi.mode(WIFI_STA);
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  dht.begin();

  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(BANGKOK_TIMEZONE);
}



void loop() {

  // Read temp & Humidity for DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(500);
    return;
  }

//  printCurrentDateTime();

  // Get current date time uid.
  String currentDateTime = getCurrentDateTime();

  Serial.print("currentDateTime=");
  Serial.println(currentDateTime);

  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  JsonObject& sensorValues = root.createNestedObject(currentDateTime);
  sensorValues["temperature"] = t;
  sensorValues["humidity"] = h;

//  JsonObject& sensorValues = jsonBuffer.createObject();
//  sensorValues["temperature"] = t;
//  sensorValues["humidity"] = h;
//
//  root[currentDateTime] = sensorValues;
  
  // append a new value to /logDHT
  String nodeString = "users/cray/devices/ht-00001/ht-00001_history/" + currentDateTime;
  Serial.print("nodeString=");
  Serial.println(nodeString);
//  String name = Firebase.push("logDHT", root);
//  String name = Firebase.push(nodeString, sensorValues);
//  Firebase.set(nodeString + "/x1", root); // failed
//  Firebase.set(nodeString + "/x1", sensorValues); // allow
//  Firebase.set(nodeString + "/" + currentDateTime, sensorValues);
  Firebase.set(nodeString, sensorValues); // allow
//  delay(2000);
//  Firebase.set(nodeString, sensorValues);
  // handle error
  if (Firebase.failed()) {
      Serial.print("pushing /logDHT failed:");
      Serial.println(Firebase.error());  
      return;
  }
  Serial.print("pushed: /logDHT/");
//  Serial.print(name);
  Serial.print(" humid: ");
  Serial.print(h);
  Serial.print(" % temperature: ");
  Serial.print(t);
  Serial.println(" Celsius");
  
  delay(5000);
}

String getCurrentDateTime() {
  String result = "";
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
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  String formattedTime = timeClient.getFormattedTime();
  
  char date_buf[20];
  sprintf(date_buf,"%04u-%02u-%02u %s", currentYear,currentMonth,monthDay, formattedTime.c_str());
  Serial.print("date_buf=");
  Serial.println(date_buf);

//  result = date_buf.c_str() + " " + formattedTime;
  result = date_buf;
  
  return result;
}

void printCurrentDateTime() {
  timeClient.update();

  unsigned long epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);  

  int currentHour = timeClient.getHours();
  Serial.print("Hour: ");
  Serial.println(currentHour);  

  int currentMinute = timeClient.getMinutes();
  Serial.print("Minutes: ");
  Serial.println(currentMinute); 
   
  int currentSecond = timeClient.getSeconds();
  Serial.print("Seconds: ");
  Serial.println(currentSecond);  

  String weekDay = weekDays[timeClient.getDay()];
  Serial.print("Week Day: ");
  Serial.println(weekDay);    

  //Get a time structure
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
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  Serial.print("Current date: ");
  Serial.println(currentDate);

  Serial.println("");
}
