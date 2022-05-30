//#define D1 5 
//#define button D1     // switch input Active High

#define D7 13 
#define button D7     // switch input Active High

#define pressed HIGH
void setup() 
{
  Serial.begin(9600);
  pinMode(button,INPUT_PULLUP);
}
void loop() 
{
  bool ReadSwitch = digitalRead(button);
  if(ReadSwitch == pressed)
  {
    Serial.println("Pressed Switch.");
    delay(500);
  }
}
