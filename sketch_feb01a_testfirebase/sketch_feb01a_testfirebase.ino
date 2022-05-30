//#include <common.h>
//#include <FirebaseFS.h>
//#include <Firebase_ESP_Client.h>
//#include <Utils.h>

#include <ESP8266WiFi.h>

#include<FirebaseArduino.h>


// Config Firebase
#define FIREBASE_HOST "asset-management-lff.firebaseio.com"
#define FIREBASE_AUTH "tyPYNCdjHQREFNsW46sha4JhOFG4WG4U7pL8iShx"

/* 2. Define the Firebase project host name and API Key */
//#define FIREBASE_HOST "PROJECT_ID.firebaseio.com"
//#define API_KEY "tyPYNCdjHQREFNsW46sha4JhOFG4WG4U7pL8iShx"

// Config connect WiFi
#define WIFI_SSID "Killing Me Softly 2G"
#define WIFI_PASSWORD "14mP455w0rd"


//Define Firebase Data objects
//FirebaseData firebaseData1;
//FirebaseData firebaseData2;

int i = 0;
String name;

void setup() {
  Serial.begin(9600);

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
}

void loop() {

  // append a new value to /temperature
  name = Firebase.pushFloat("temperature", i);
  if (Firebase.failed()) {
      Serial.print("pushing /temperature failed:");
      Serial.println(Firebase.error());  
      return;
  }
  Serial.print("pushed: /temperature/");
  Serial.println(name);
  
  Firebase.setInt("number", i);
  Serial.print("number: " + i);
  if (Firebase.failed()) {
      Serial.print("set /number failed:");
      Serial.println(Firebase.error());  
      return;
  }
  Serial.print("set /number to ");
  Serial.println(Firebase.getInt("number"));
  
  i++;
  delay(2000);
}
