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

String gMacAddress = "";

void setup() {
  Serial.begin(115200);
  // initialize the LED pin as an output.
  pinMode(13, OUTPUT);
  
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
  delay(1000);
  Serial.println(".");
  Serial.println("sketch_mar21b_eeprom_clear");
  Serial.println("Erasing");

  if ( RESET_EEPROM ) {
    for (int i = 0; i < 512; i++) {
      EEPROM.write(i, 0);
      Serial.print(".");
    }
    EEPROM.commit();
    delay(500);

    EEPROM.end();
    Serial.println("");
    Serial.println("EEPROM erased!!");
  }

  
//  for (int i = 0 ; i < EEPROM.length() ; i++) {
//    EEPROM.write(i, 0);
//  }

  // turn the LED on when we're done
  digitalWrite(13, HIGH);

  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  gMacAddress = WiFi.macAddress();
  Serial.print("gMacAddress:  ");
  Serial.println(gMacAddress);
  
}

void loop() {
  /** Empty loop. **/
}
