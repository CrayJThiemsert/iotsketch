/***********************|
 * WiFI สำหรับ ESP8266   |
 ***********************/
    #include <ESP8266WiFi.h>

/************************|
 * รหัส WiFi ที่ต้องเชื่อมต่อ   |
 ************************/
    char* ssid     = "Killing Me Softly 2G";
    char* password = "14mP455w0rd";


/************************|
 * ตั้งค่า Static IP ตรงนี้   |
 ************************/
    IPAddress local_IP(192, 168, 1, 153);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 254, 0);
    IPAddress primaryDNS(8, 8, 8, 8);

/****************************************************************|
 * ตั้งค่าเซิฟเวอร์ พอร์ต 80 (ถ้าตั้งเลขอื่น ต้องเข้าด้วย http://{IP}:{เลขport}  |
 ****************************************************************/
    WiFiServer server(80);



/*******************************|
 * ตัวแปรที่เกี่ยวข้องกับการเปิดปิด LED   |
 *******************************/
    // Variable to store the HTTP request
    String header;
    
    // Auxiliar variables to store the current output state
    String output2State = "off";
    
    // Assign output variables to GPIO pins
    const int output2 = 2;
    

void setup() {

  
  /*******************************|
   * ตัวแปรที่เกี่ยวข้องกับการเปิดปิด LED   |
   *******************************/
      pinMode(output2, OUTPUT);
      
  
  /***************************************|
   * เริ่มการสื่อสารแบบ Serial baudrate 115200 |
   ***************************************/
      Serial.begin(115200);
      Serial.println("Started \n\n\n");



  /************************************************|
   * เริ่มตั้งค่า WiFi ด้วย Static ip ที่ตั้งไว้ด้วยคำสั่ง config |
   * และต่อ WiFi ด้วยคำสั่ง begin                      |
   ***********************************************/
      //Format : void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);
      WiFi.config(local_IP, primaryDNS, gateway, subnet);
 
      Serial.print("Connecting to ");
      Serial.println(ssid);

      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(50);
        Serial.print(".");
      }

      // แสดงค่า Ip หลังต่อเชื่อมต่อ WiFi สำเร็จแล้ว
      Serial.println("");
      Serial.println("WiFi connected.");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());

  /****************|
   * เริ่ม Server    |
   ****************/
      server.begin();
}

void loop(){
  WiFiClient client = server.available();   // รอรับผู้ใช้ ถ้ามีการเรียกใช้ IP เรา object client จะให้ค่า true

  if (client) {                             // ถ้ามีผู้ใช้เรียกเข้ามา
    Serial.println("New Client.");          // พิมพ์คำสั่ง New client
    String currentLine = "";                
    while (client.connected()) {            
      if (client.available()) {             
        char c = client.read();             // อ่านค่าจากผู้ใช้ (เป็นข้อมูลช่องทางที่ผู้ใช้ติดต่อมา เช่น browser, ping , etc)
        Serial.write(c);                    // แสดงค่าพวกนั้นลงบน Serial monitor 
        header += c;
        if (c == '\n') {                    // ถ้ามีการขึ้นบรรทัดใหม่แสดงว่าจบการ request ของผู้ใช้แล้ว ให้ตอบกลับด้วยหน้าเว็ปกลับไป

          if (currentLine.length() == 0) {
            // HTTP headers จะเริ่มต้นด้วย code (เช่น HTTP/1.1 200 OK)
            // และขึ้นบรรทัดใหม่ตามด้วยเนื้อหา html ข้างใน
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // เปิดปิด LED
            if (header.indexOf("GET /2/on") >= 0) {
              Serial.println("GPIO 2 on");
              output2State = "on";
              digitalWrite(output2, LOW);
            } else if (header.indexOf("GET /2/off") >= 0) {
              Serial.println("GPIO 2 off");
              output2State = "off";
              digitalWrite(output2, HIGH);
            }
            
            // แสดงค่า HTML ออกหน้าเว็ป
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            client.println("<body><h1>ArduinoNa Web Server</h1>");
            
            // แสดงสถานะของ GPIO
            client.println("<p>GPIO 2 - State " + output2State + "</p>");
            if (output2State=="off") {
              client.println("<p><a href=\"/2/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/2/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            
            client.println("</body></html>");
            
            // จบการส่งเนื้อหา HTML ด้วยการขึ้นบรรทัดใหม่
            client.println();
            break;
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') {  
          currentLine += c;  
        }
      }
    }
    // เคลียร์ค่า header
    header = "";
    // ปิดการเชื่อมต่อ
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
