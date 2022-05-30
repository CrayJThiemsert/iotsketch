#include <ESP8266WiFi.h>

#include<FirebaseArduino.h>
#include <DHT.h>


// Config Firebase
#define FIREBASE_HOST "asset-management-lff.firebaseio.com"
#define FIREBASE_AUTH "tyPYNCdjHQREFNsW46sha4JhOFG4WG4U7pL8iShx"

//// Config connect WiFi Cray Home
//#define WIFI_SSID "Killing Me Softly 2G"
//#define WIFI_PASSWORD "14mP455w0rd"

//// Config connect WiFi Intec Office
//#define WIFI_SSID "Intec 02"
//#define WIFI_PASSWORD "solar100"

//// Config connect ChangMalayCondo_2
//#define WIFI_SSID "ChangMalayCondo_2"
//#define WIFI_PASSWORD "marktoberdorf"

// Config connect Chang Malay Office
#define WIFI_SSID "Chang Malay Office"
#define WIFI_PASSWORD "marktoberdorf"

// Config DHT
#define DHTPIN 4
//#define DHTTYPE DHT11
#define DHTTYPE DHT22   // DHT 22 

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
//  float h = dht.readHumidity();
//  float t = dht.readTemperature();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
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
  Serial.println(name);

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F(" C "));
  Serial.print(f);
  Serial.print(F(" F  Heat index: "));
  Serial.print(hic);
  Serial.print(F(" C "));
  Serial.print(hif);
  Serial.println(F(" F"));
  
  delay(5000);
}
