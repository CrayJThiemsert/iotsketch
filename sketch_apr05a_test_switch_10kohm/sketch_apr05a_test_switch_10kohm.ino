const int resetButton = 16;
int temp = 0;

int resetCount = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(resetButton, INPUT);

  pinMode(2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  temp = digitalRead(resetButton);
  
  if(temp == HIGH) {
    if(resetCount == 3) {
      blinkLED();
      resetCount = 0;
    } else {
      Serial.print("Reset button ON resetCount=");
      Serial.println(resetCount);                           
      digitalWrite(2, HIGH);
      resetCount = resetCount + 1;
      
      delay(1000);
    }
  } else {
    Serial.print("Reset button OFF resetCount=");
    Serial.println(resetCount);
    digitalWrite(2, LOW);
    delay(1000);
  }

}

void blinkLED() {
  // Blink 5 times 
  for (int i = 0; i < 5; i++) {
    digitalWrite(2, HIGH);
    delay(200);
    digitalWrite(2, LOW);
    delay(200);
  }
}
