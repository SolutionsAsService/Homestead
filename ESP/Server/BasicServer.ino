#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Replace with your network credentials
const char* ssid = "emeraldcity";
const char* password = "spacemonkeys";

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

  // Initialize OLED display (using 0x3C as the I2C address)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Show initial message
  display.setCursor(0, 0);
  display.println("Connecting to Wi-Fi...");
  display.display();

  // Begin Wi-Fi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");

  // Increase the number of attempts to wait longer (here up to 20 seconds)
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 40) {
    delay(500);
    Serial.print(".");
    attempt++;

    // Update OLED with attempt count and current WiFi.status() code
    display.clearDisplay();
    display.setCursor(2, 2);
    display.print("Attempt ");
    display.print(attempt);
    display.print(": ");
    display.println(WiFi.status());
    display.display();
  }
  
  display.clearDisplay();
  if(WiFi.status() == WL_CONNECTED) {
    display.println("Wi-Fi Connected!");
    display.print("IP: ");
    display.println(WiFi.localIP());
    Serial.println("\nWi-Fi Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    display.println("Wi-Fi Connection Failed!");
    Serial.println("\nWi-Fi Connection Failed!");
  }
  display.display();
  delay(2000);

  // Set up the web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Listen for client connections
  server.handleClient();
}
