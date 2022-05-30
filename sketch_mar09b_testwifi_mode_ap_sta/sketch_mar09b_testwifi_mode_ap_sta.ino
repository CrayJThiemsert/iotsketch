#include <ESP8266WiFi.h>


// Config connect WiFi
#define WIFI_SSID "Killing Me Softly 2G"
#define WIFI_PASSWORD "14mP455w0rd"

#define THE_NODE_SSID "theNode_001"
#define THE_NODE_PASSWORD "14mP455w0rd"

int i = 0;
String name;

void setup() {
  Serial.begin(9600);

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


  IPAddress ap_local_ip = {192,168,1,144};   // Set up IP
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
}

void loop() {

  
}
