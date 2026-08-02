#include <WiFi.h>

const char* ssid = "TP-Link_25CF";
const char* password = "Happylife!505";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Off initially

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.begin();
}

void loop() {
  WiFiClient client = server.available();   

  if (client) {
    String currentLine = "";
    String requestString = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        requestString += c; // Capture the full request
        
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Check what the user requested before sending the HTML response
            if (requestString.indexOf("GET /on") >= 0) {
              Serial.println("Command: Turn ON");
              digitalWrite(LED_BUILTIN, HIGH); // Change to HIGH if your LED logic is reversed
            } else if (requestString.indexOf("GET /off") >= 0) {
              Serial.println("Command: Turn OFF");
              digitalWrite(LED_BUILTIN, LOW); // Change to LOW if your LED logic is reversed
            }

            // Send HTTP response
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>");
            client.println("<body><h1>ESP Light Control Test</h1>");
            client.println("<p><a href=\"/on\"><button>ON</button></a></p>");
            client.println("<p><a href=\"/off\"><button>OFF</button></a></p>");
            client.println("</body></html>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
  }
}