/*
 * EEPROM Write
 *
 * Stores values and string to EEPROM.
 * These values and string will stay in the EEPROM when the board is
 * turned off and may be retrieved later by another sketch.
 */

#include <EEPROM.h>

// the current address in the EEPROM (i.e. which byte
// we're going to write to next)
int addr = 0;

#define WWW_URL 0x0F

void setup()
{
  EEPROM.begin(512);  //Initialize EEPROM

  Serial.begin(9600); //Serial communication to display data

  // write appropriate byte of the EEPROM.
  // these values will remain there when the board is
  // turned off.

  /*
  EEPROM.write(addr, 'A');    //Write character A
  addr++;                      //Increment address
  EEPROM.write(addr, 'B');    //Write character B
  addr++;                      //Increment address
  EEPROM.write(addr, 'C');    //Write character C
  addr++;                      //Increment address
  EEPROM.write(addr, 'D');    //Write character D
  */

  //Write string to eeprom
  String www = "www.cray.com";
  for(int i=0;i<www.length();i++) //loop upto string lenght www.length() returns length of string
  {
    EEPROM.write(WWW_URL+i,www[i]); //Write one by one with starting address of 0x0F
  }
  EEPROM.commit();    //Store data to EEPROM

  delay(1000);

  // Read wrote value
  Serial.println("Reading...");
  read_eeprom();
  
}

void read_eeprom()
{
  // read appropriate byte of the EEPROM.  
  Serial.println("Reading:"); //Goto next line, as ESP sends some garbage when you reset it  
  /*
  Serial.print(char(EEPROM.read(addr)));    //Read from address 0x00
  addr++;                      //Increment address
  Serial.print(char(EEPROM.read(addr)));    //Read from address 0x01
  addr++;                      //Increment address
  Serial.println(char(EEPROM.read(addr)));    //Read from address 0x02
  addr++;                      //Increment address
  Serial.println(char(EEPROM.read(addr)));    //Read from address 0x03
  */

  //Read string from eeprom
  String www;   
  //Here we dont know how many bytes to read it is better practice to use some terminating character
  //Lets do it manually www.circuits4you.com  total length is 20 characters
  for(int i=0;i<20;i++) 
  {
    www = www + char(EEPROM.read(WWW_URL+i)); //Read one by one with starting address of 0x0F    
  }  

  Serial.println(www);  //Print the text on serial monitor
}

void loop()
{
  //We dont have anything in loop as EEPROM writing is done only once
  delay(10000);   

  Serial.println("In Loop");
  read_eeprom();
}
