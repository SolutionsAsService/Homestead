#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Replace with your network credentials
const char* ssid = "";
const char* password = "";

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Web server running on port 80
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

// Handle root URL "/"
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void setup() {
  Serial.begin(115200);

  // Initialize OLED display
  if(!display.begin(SSD1306_I2C_ADDRESS, OLED_RESET)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();

  // Connect to Wi-Fi
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting to Wi-Fi...");
  display.display();
  WiFi.begin(ssid, password);
  
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(".");
    attempt++;
  }
  
  display.clearDisplay();
  if(WiFi.status() == WL_CONNECTED) {
    display.println("Wi-Fi Connected!");
    display.print("IP: ");
    display.println(WiFi.localIP());
  } else {
    display.println("Wi-Fi Connection Failed!");
  }
  display.display();

  // Start the server
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}
