#include "ESP8266WiFi.h"
#include "DHT.h"

#define DHT11PIN 4 // D7        // define the digital I/O pin
#define DHT11TYPE DHT11         // DHT 11 
DHT dht(DHT11PIN, DHT11TYPE);

void setup() {
  // put your setup code here, to run once:
//  Serial.begin(115200);
  Serial.begin(9600);
  delay(100);
  Serial.setTimeout(2000);

  Serial.println("Waiting for serial initialize");

  // Wait for serial to initialize.
  while(!Serial) { }

  dht.begin();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(500);
    return;
  }
 
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C"); 

//  String hs="Hum: "+String((float)h)+" % ";
//  String ts="Temp: "+String((float)t)+" C ";

  
  
  // Deep sleep mode for 30 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
  Serial.println("I'm awake, but I'm going into deep sleep mode for 10 seconds");
  ESP.deepSleep(10e6); 

  Serial.println("End 10sec");
  
  // Deep sleep mode until RESET pin is connected to a LOW signal (for example pushbutton or magnetic reed switch)
  //Serial.println("I'm awake, but I'm going into deep sleep mode until RESET pin is connected to a LOW signal");
  //ESP.deepSleep(0); 

  delay(100);
  Serial.println("Setup done");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("In loop");

  // Wait a bit before scanning again
  delay(5000);
}
