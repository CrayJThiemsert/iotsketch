

/* This code is to use with ToF10120 Laser Range Sensor to measure distance in (mm)and (inches) and shows it on the Serial monitor and also on the I2C LCD using I²c inteface
 * Modified and adapted from a code found on some dodgy chinese website
 * Refer to https://www.electroniclinic.com/ for more details
 */
 
#include <Wire.h>
#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
//#include <SimpleTimer.h>
#include <SimpleTimer.h>
 
SimpleTimer timer;
 
unsigned char ok_flag;
unsigned char fail_flag;
 
unsigned short lenth_val = 0;
unsigned char i2c_rx_buf[16];
unsigned char dirsend_flag=0;
 
int x_mm; // distance in millimeters
float y_inches; // distance in inches
 
// for the OLED display
 
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
 
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
int relay = 13; 
int relay_flag = 0; 
 
void setup() {
  Wire.begin(); 
//  Serial.begin(9600,SERIAL_8N1); 
  Serial.begin(9600); 
//  Serial.begin(115200); 
//  pinMode(relay, OUTPUT); 
//  digitalWrite(relay, LOW);
//  printf_begin(); 
//  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
//  display.clearDisplay();
//  display.setTextColor(WHITE);         
 
//timer.setInterval(500L, display_measurement);
  timer.setInterval(3000);
}
 
void loop() {
  
//timer.run(); // Initiates SimpleTimer
  if (timer.isReady()) {                    // Check is ready a second timer
     Serial.println("Called every 3 sec");
 
     x_mm = ReadDistance();
     Serial.print(x_mm);
     Serial.println(" mm");
   
     // You can convert millimeters to inches in one of two ways: divide the number of millimeters by 25.4, or multiply the number of millimeters by 0.0394
     y_inches = x_mm * 0.0394;
     Serial.print(y_inches); 
     Serial.println(" inches");
 
    if( (y_inches > 10 ) && (relay_flag == 0))
    {
      digitalWrite(relay, LOW); 
      relay_flag = 1; 
    }
 
    if( (y_inches <= 5 ) && (relay_flag == 1))
    {
      digitalWrite(relay, HIGH); 
      relay_flag = 0; 
    }

    timer.reset(); 
  } // timer
 
}
 
int serial_putc( char c, struct __file * )
{
  Serial.write( c );
  return c;
}
 
//void printf_begin(void)
//{
//  fdevopen( &serial_putc, 0 );
//}
 
 
 
void SensorRead(unsigned char addr,unsigned char* datbuf,unsigned char cnt) 
{
  unsigned short result=0;
  // step 1: instruct sensor to read echoes
  Wire.beginTransmission(82); // transmit to device #82 (0x52), you can also find this address using the i2c_scanner code, which is available on electroniclinic.com
  // the address specified in the datasheet is 164 (0xa4)
  // but i2c adressing uses the high 7 bits so it's 82
  Wire.write(byte(addr));      // sets distance data address (addr)
  Wire.endTransmission();      // stop transmitting
  // step 2: wait for readings to happen
  delay(1);                   // datasheet suggests at least 30uS
  // step 3: request reading from sensor
  Wire.requestFrom(82, cnt);    // request cnt bytes from slave device #82 (0x52)
  // step 5: receive reading from sensor
  if (cnt <= Wire.available()) { // if two bytes were received
    *datbuf++ = Wire.read();  // receive high byte (overwrites previous reading)
    *datbuf++ = Wire.read(); // receive low byte as lower 8 bits
  }
}
 
int ReadDistance(){
    SensorRead(0x00,i2c_rx_buf,2);
    lenth_val=i2c_rx_buf[0];
    lenth_val=lenth_val<<8;
    lenth_val|=i2c_rx_buf[1];
    delay(300); 
    return lenth_val;
}
 
//void display_measurement()
//{
//     // display on Oled display
// 
//   // Oled display
//  display.clearDisplay();
//  display.setTextSize(2);
//  display.setCursor(0,0); // column row
//  display.print("mm:");
// 
//  display.setTextSize(2);
//  display.setCursor(55, 0);
//  display.print(x_mm);
// 
//    display.setTextSize(2);
//  display.setCursor(0,30);
//  display.print("In:");
// 
//  display.setTextSize(2);
//  display.setCursor(60, 30);
//  display.print(y_inches);
//  display.setCursor(95, 50);
// 
// display.display();
//}
