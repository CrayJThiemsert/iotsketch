#include <ESP8266WiFi.h>


// Config connect WiFi
#define WIFI_SSID "Killing Me Softly 2G"
#define WIFI_PASSWORD "14mP455w0rd"

#define THE_NODE_SSID "theNode_001"
#define THE_NODE_PASSWORD "14mP455w0rd"

int i = 0;
String name;

WiFiServer wifiServer(80);

void setup() {
  Serial.begin(9600);

  delay(100);

  WiFi.mode(WIFI_AP_STA);
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("STA connected: ");
  Serial.println(WiFi.localIP());


  IPAddress ap_local_ip = {192,168,1,154};   // Set up IP
  IPAddress gateway={192,168,1,1};      // Set up Gateway
  IPAddress subnet={255,255,255,0};     // Set up Subnet
  WiFi.softAP(THE_NODE_SSID, THE_NODE_PASSWORD);           // Set Soft SSID
  WiFi.softAPConfig(ap_local_ip,gateway,subnet); // Set up to module  

  Serial.println(""); 
  Serial.println("WiFi connected");     // Display connected success
  Serial.println("AP IP address: "); 
  Serial.println(WiFi.softAPIP());       // Show ESP8266's IP Address

  uint8_t macAddr[6];
  WiFi.softAPmacAddress(macAddr);
  Serial.printf("MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  Serial.printf("MAC address String = %s\n", WiFi.softAPmacAddress().c_str());

  wifiServer.begin();
}

void loop() {
//  webDisplayOnly();
  webButtonsCommand();
}

void webDisplayOnly() {
  // put your main code here, to run repeatedly:

  // Check if a client has connected
  WiFiClient client = wifiServer.available();
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

  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  client.println("Battery Voltage =");
  client.print(THE_NODE_SSID);
  client.println("<br>");

  
  client.println("Updatedxxx");
  
  client.println("--------");
  
  client.println("Battery Full");
  
     
  client.println("<br><br>");
  client.println("<a href=\"/bat=ON\"\"><button>Status</button></a><br />");  
  client.println("</html>");
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
}

void webButtonsCommand() {
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
