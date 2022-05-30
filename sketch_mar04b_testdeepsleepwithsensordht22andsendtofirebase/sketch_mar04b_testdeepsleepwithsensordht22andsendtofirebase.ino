/*
 * ESP8266 Deep sleep mode example with 
 * - blink once 
 * - read humid and teperature 
 * - send read data to firebase
 * 
 * Jirachai Thiemsert 
 * 04-Mar-2021 15:31:53
 * Complete Project Details cray.j.thiemsert@gmail.com
 */

#include <DHT.h>
#include <ESP8266WiFi.h>
#include<FirebaseArduino.h>

// Config Firebase
#define FIREBASE_HOST "asset-management-lff.firebaseio.com"
#define FIREBASE_AUTH "tyPYNCdjHQREFNsW46sha4JhOFG4WG4U7pL8iShx"

//// Config connect WiFi Cray Home
//#define WIFI_SSID "Killing Me Softly 2G"
//#define WIFI_PASSWORD "14mP455w0rd"

// Config connect WiFi Intec Office
#define WIFI_SSID "Intec 02"
#define WIFI_PASSWORD "solar100"

//// Config connect ChangMalayCondo_2
//#define WIFI_SSID "ChangMalayCondo_2"
//#define WIFI_PASSWORD "marktoberdorf"

// Config DHT
#define DHTPIN 4
//#define DHTTYPE DHT11
#define DHTTYPE DHT22   // DHT 22 

String name;
DHT dht(DHTPIN, DHTTYPE);
 
void setup() {
  // =================================================================
  // 1. Woke up process part
  // =================================================================
  Serial.begin(115200);
  Serial.println("Woke up!!");
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while(!Serial) { }

  
  // =================================================================
  // 2. Working process part
  // =================================================================
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  delay(500);
  
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW

  WiFi.mode(WIFI_STA);
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // Reading temperature or humidity takes about 250 milliseconds!
  dht.begin();
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  Serial.println("Reading Humidity and Temperature:");
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

  // Print out result to serial monitor
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");
  Serial.print("Temperature Celsius: ");
  Serial.print(t);
  Serial.print(" *C"); 
  Serial.print(" ..... Temperature Fahrenheit: ");
  Serial.print(f);
  Serial.println(" *F");

  // --------------------------
  // Upload data to Firebase
  // --------------------------
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
  // --------------------------

  
  delay(1000);                       // wait for a second

  // =================================================================

  
  // =================================================================
  // 3. Deep Sleep process part
  // =================================================================
  // Deep sleep mode for 30 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
//  Serial.println("I'm awake, but I'm going into deep sleep mode for 30 seconds");
  Serial.println("Deep Sleep 30 seconds");
  ESP.deepSleep(10e6); 
  
  // Deep sleep mode until RESET pin is connected to a LOW signal (for example pushbutton or magnetic reed switch)
  //Serial.println("I'm awake, but I'm going into deep sleep mode until RESET pin is connected to a LOW signal");
  //ESP.deepSleep(0); 
}

void loop() {
}
