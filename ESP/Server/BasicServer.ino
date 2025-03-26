#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Create a web server object that listens on port 80
WebServer server(80);

// HTML content for the webpage
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>ESP32 Web Server</title>
    <style>
      body { font-family: Arial; text-align: center; margin-top: 50px; }
      h1 { color: #007ACC; }
    </style>
  </head>
  <body>
    <h1>Hello from ESP32!</h1>
    <p>This is a simple webpage served from an ESP32.</p>
  </body>
</html>
)rawliteral";

// Function to handle the root URL "/"
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Connect to Wi-Fi network
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Define routing
  server.on("/", handleRoot);
  
  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Listen for client connections
  server.handleClient();
}
