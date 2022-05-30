#include <ESP8266WiFi.h>

#include<FirebaseArduino.h>
#include <DHT.h>


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

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["temperature"] = t;
  root["humidity"] = h;
  
  // append a new value to /logDHT
  String name = Firebase.push("logDHT", root);
  // handle error
  if (Firebase.failed()) {
      Serial.print("pushing /logDHT failed:");
      Serial.println(Firebase.error());  
      return;
  }
  Serial.print("pushed: /logDHT/");
  Serial.print(name);
  Serial.print(" humid: ");
  Serial.print(h);
  Serial.print(" % temperature: ");
  Serial.print(t);
  Serial.println(" Celsius");
  
  delay(5000);
}
