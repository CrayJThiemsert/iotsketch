/*
 * ESP8266 Deep sleep mode example with 
 * - blink once 
 * - read humid and teperature 
 * 
 * Jirachai Thiemsert 
 * 2021
 * Complete Project Details cray.j.thiemsert@gmail.com
 */

#include <DHT.h>

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
