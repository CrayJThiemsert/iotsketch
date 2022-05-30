#include <EEPROM.h>


#define PHONE1 "+420123456789" //phone number 1
#define PHONE2 "+420987654321" //phone number 2
#define PHONE3 "+420555554603" //phone number 2
#define passw0 "1234" //phone number 2

char thepass[5] = {'\0'};
char phoneA[14] = {'\0'};
char phoneB[14] = {'\0'};
char phoneC[14] = {'\0'};

void setup() {
Serial.begin(9600);

//just temporary for 1st time down  
  EEPROM.put(0,passw0);
  EEPROM.put(111,PHONE1);
  EEPROM.put(222,PHONE2);
  EEPROM.put(333,PHONE3);

 //just temporary for 1st time up
  Serial.println("get");
  Serial.println(thepass);
  Serial.println(phoneA);
  Serial.println(phoneB);
  Serial.println(phoneC);
  Serial.println("");
  Serial.println("put");
  Serial.println(passw0);
  Serial.println(PHONE1);
  Serial.println(PHONE2);
  Serial.println(PHONE3);
  Serial.println("");
  


  EEPROM.get(0,thepass);
  EEPROM.get(111,phoneA);
  EEPROM.get(222,phoneB);
  EEPROM.get(333,phoneC);


  Serial.println("Pass");
  Serial.print(thepass);
  Serial.println();
  Serial.println("Number1");
  Serial.println(phoneA);
  Serial.println();
  Serial.println("Number2");
  Serial.println(phoneB);
  Serial.println("Number3");
  Serial.println(phoneC);
  Serial.println();
}
void loop()
{}
