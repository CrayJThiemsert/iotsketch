#include <EEPROM.h>

struct MyObject {
  float field1;
  byte field2;
  char name[10];
  String uid;
};

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  String myString="Arduino UNO";
  char myBoard[15];

  strcpy(myBoard,myString.c_str());
  EEPROM.put(0,myBoard);

  Serial.println("Written String data type!");
  
//  char theBoard[15];
//  EEPROM.get(0,theBoard);

//  EEPROM.begin(512);

  
//  float f = 123.456f;  //Variable to store in EEPROM.
//  int eeAddress = 0;   //Location we want the data to be put.
//
//
//  //One simple call, with the address first and the object second.
//  EEPROM.put(eeAddress, f);
//
//  Serial.println("Written float data type!");
//
//  /** Put is designed for use with custom structures also. **/
//
//  //Data to store.
//  MyObject customVar = {
//    3.14f,
//    789,
//    "Working!",
//    "2021"
//  };
//
//  eeAddress += sizeof(float); //Move address to the next byte after float 'f'.
//
//  EEPROM.put(eeAddress, customVar);
//  Serial.print("Written custom data type! \n\nView the example sketch eeprom_get to see how you can retrieve the values!");
}

void loop() {   /* Empty loop */ }
