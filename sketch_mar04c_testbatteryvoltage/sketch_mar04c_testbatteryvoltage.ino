/*****************************************************
 * Date: 18 june 2018
 * Written by: Usman Ali Butt
 * Property off: microcontroller-project.com
 * ***************************************************/
#include <ESP8266WiFi.h>
 
//const char* ssid = "Intec 02"; // "Your SSID";
//const char* password = "solar100"; // "Your Wifi Password";

const char* ssid = "Killing Me Softly 2G"; // "Your SSID";
const char* password = "14mP455w0rd"; // "Your Wifi Password";

//// Config connect WiFi Cray Home
//#define WIFI_SSID "Killing Me Softly 2G"
//#define WIFI_PASSWORD "14mP455w0rd"

// Config connect WiFi Intec Office
//#define WIFI_SSID "Intec 02"
//#define WIFI_PASSWORD "solar100"
 
int BAT= A0;              //Analog channel A0 as used to measure battery voltage
// float RatioFactor=5.714;  //Resistors Ration Factor
// float RatioFactor=1.643;  //Resistors Ration Factor
// float RatioFactor=1.50;  //Resistors Ration Factor
// float RatioFactor=1.10;  //Resistors Ration Factor
// float RatioFactor=1.08;  //Resistors Ration Factor
float RatioFactor=1.07;  //Resistors Ration Factor
WiFiServer server(80);
 
void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin(); // Start the server
  Serial.println("Server started");
  // Print the IP address on serial monitor
  Serial.print("Use this Wifi SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("Use this Wifi Mac Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");    //URL IP to be typed in mobile/desktop browser
  Serial.print(WiFi.localIP());
  
  Serial.println("/");
}
 
void loop() {
  int value = LOW;
  float Tvoltage=0.0;
  float Vvalue=0.0,Rvalue=0.0;
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Match the request
  
  if (request.indexOf("/bat=ON") != -1)  {
    /////////////////////////////////////Battery Voltage//////////////////////////////////  
    for(unsigned int i=0;i<10;i++){
      Vvalue=Vvalue+analogRead(BAT);         //Read analog Voltage
      delay(5);                              //ADC stable
    }
    Vvalue=(float)Vvalue/10.0;            //Find average of 10 values
    Rvalue=(float)(Vvalue/1024.0)*5;      //Convert Voltage in 5v factor
    Tvoltage=Rvalue*RatioFactor;          //Find original voltage by multiplying with factor
    /////////////////////////////////////Battery Voltage//////////////////////////////////
    value = HIGH;
  }
 
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  client.println("Battery Voltage =");
  client.print(Tvoltage);
  client.println("<br>");

  if(value == HIGH) {
    client.println("Updated");
  } else {
    client.print("Not Updated");
  }
  client.println("--------");
  if(Tvoltage<=5){
    client.println("Battery dead OR disconnected");
    }
  else if(Tvoltage>5 && Tvoltage<=10){
    client.println("Need Imediate recharge");
    }
  else if(Tvoltage>10 && Tvoltage<=12){
    client.println("Recharge");
    }
  else{
      client.println("Battery Full");
      }
     
  client.println("<br><br>");
  client.println("<a href=\"/bat=ON\"\"><button>Status</button></a><br />");  
  client.println("</html>");
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
 
}
