/*
 * EEPROM Clear
 *
 * Sets all of the bytes of the EEPROM to 0.
 * Please see eeprom_iteration for a more in depth
 * look at how to traverse the EEPROM.
 *
 * This example code is in the public domain.
 */

#include <EEPROM.h>

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

// Set to true to reset eeprom before to write something
#define RESET_EEPROM true
#define EEPROM_ADDR 512
#define EEPROM_TRANSACTION_ADDR 1024

String gMacAddress = "";

struct WeatherHistory {
  String uid;
  float humidity;
  float temperature;
  float readVoltage;
};

WeatherHistory weatherData = {"2021-05-09 10:45:14", 65.7, 27.5, 3.05};
WeatherHistory weatherData2 = {"2021-04-19 16:33:20", 65.70, 127.52, 3.05};
WeatherHistory weatherData3;

void setup() {
  Serial.begin(115200);
  // initialize the LED pin as an output.
  pinMode(13, OUTPUT);

  Serial.println("");
  // Writing data testing ================= start
//  Serial.print("size of weatherData=");
//  Serial.println(sizeof(weatherData));
//  int EEAddr = EEPROM_TRANSACTION_ADDR;
//
//  Serial.print("EEAddr=");
//  Serial.println(EEAddr);
//  EEPROM.put(EEAddr,weatherData); EEAddr += sizeof(weatherData);
//
//  Serial.print("size of weatherData2=");
//  Serial.println(sizeof(weatherData2));
//  Serial.print("EEAddr2=");
//  Serial.println(EEAddr);
//  EEPROM.put(EEAddr,weatherData2); EEAddr += sizeof(weatherData2);
  // Writing data testing ================= end

  // Reading data testing ================= start
  Serial.println("Get data2->3...");
  EEPROM.get(1024, weatherData3);
  show_weatherdata(&weatherData3);
  Serial.println("Done...");
  // Reading data testing ================= start
  /***
    Iterate through each byte of the EEPROM storage.

    Larger AVR processors have larger EEPROM sizes, E.g:
    - Arduno Duemilanove: 512b EEPROM storage.
    - Arduino Uno:        1kb EEPROM storage.
    - Arduino Mega:       4kb EEPROM storage.

    Rather than hard-coding the length, you should use the pre-provided length function.
    This will make your code portable to all AVR processors.
  ***/
//  EEPROM.begin(EEPROM_ADDR);
//  delay(1000);
//  Serial.println("Erasing");
//
//  if ( RESET_EEPROM ) {
//    for (int i = 0; i < EEPROM_ADDR; i++) {
//      EEPROM.write(i, 0);
//      Serial.print(".");
//    }
//    EEPROM.commit();
//    delay(500);
//
//    EEPROM.end();
//    Serial.println("");
//    Serial.println("EEPROM erased!!");
//  }
//
//  
////  for (int i = 0 ; i < EEPROM.length() ; i++) {
////    EEPROM.write(i, 0);
////  }
//
//  // turn the LED on when we're done
//  digitalWrite(13, HIGH);
//
//  Serial.print("ESP Board MAC Address:  ");
//  Serial.println(WiFi.macAddress());
//
//  gMacAddress = WiFi.macAddress();
//  Serial.print("gMacAddress:  ");
//  Serial.println(gMacAddress);
  
}

void loop() {
  /** Empty loop. **/
}

void show_weatherdata(WeatherHistory *p) {
  Serial.println("");
  Serial.print("uid="); Serial.println(p->uid);
  Serial.print("humidity "); Serial.println(p->humidity);
  Serial.print("temperature "); Serial.println(p->temperature);
  Serial.print("readVoltage "); Serial.println(p->readVoltage);
  Serial.println("");
}
