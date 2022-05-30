#define ADC_PIN  A0 // Analog input pin

int avgcnt = 10;
float vout, avg, filterWeight = 64.0, corrFactor = 1.0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  getADCVoltage(5.4); // With a 220k resistor in row 
  delay(1000);
}

float getADCVoltage(float maxVoltage) {
  avg = analogRead(ADC_PIN);
  
  for (int i = 0; i < avgcnt; i++) {
    avg = avg + (analogRead(ADC_PIN) - avg) / filterWeight;
    delay(100);
  }

  vout = ((avg / 1024.0) * maxVoltage) * corrFactor;
  if(vout < 0.10) {
    vout = 0.00;
  }

  Serial.print("Battery: ");
  Serial.print(vout);
  Serial.println(" V");
  
  return vout;
}
