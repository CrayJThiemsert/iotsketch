#include <ESP8266WiFi.h>

//// Config connect WiFi Cray Home
#define WIFI_SSID "Killing Me Softly 2G"
#define WIFI_PASSWORD "14mP455w0rd"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WiFiServer wifiServer(80);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  delay(1000);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting..");
  }

  Serial.print("Connected to WiFi. IP:");
  Serial.println(WiFi.localIP());

  wifiServer.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  WiFiClient client = wifiServer.available();
  String command = "";

  if (client) {

    while (client.connected()) {

      while (client.available()>0) {
        char c = client.read();
        if (c == '\n') {
          break;
        }
        command += c;
        Serial.write(c);
      }
      
      if (command == "POWER") {
        
        Serial.println("We got the power !!");
      }else if (command == "TEMPUP") {
        
        Serial.println("We got the power !!");
      }else if (command == "TEMPDOWN") {
        
        Serial.println("We got the power !!");
      }else if (command == "FAN") {
        
        Serial.println("We got the power !!");
      }else if (command == "MODE") {

        Serial.println("We got the power !!");
      }

      command = "";
      delay(10);
    }

    client.stop();
    Serial.println("Client disconnected");
  }
}
