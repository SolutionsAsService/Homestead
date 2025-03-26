#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Replace with your Wi-Fi credentials
const char* ssid = "emeraldcity";
const char* password = "spacemonkeys";

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32  // Using a 32-pixel tall display for this example
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Web server running on port 80
WebServer server(80);

int hallValue = 0; // Global variable to store hall sensor data

// HTML page for web display
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset='utf-8'>
    <title>ESP32 Edge Sensor</title>
    <style>
      body { font-family: Arial; text-align: center; margin-top: 50px; }
      h1 { color: #007ACC; }
      p { font-size: 20px; color: #333; }
    </style>
  </head>
  <body>
    <h1>ESP32 Edge Sensor</h1>
    <p>Hall Sensor Value: <span id='hall'>0</span></p>
    <script>
      setInterval(() => {
        fetch('/hall').then(response => response.text()).then(data => {
          document.getElementById('hall').innerText = data;
        });
      }, 1000);
    </script>
  </body>
</html>
)rawliteral";

// Handler for root URL "/"
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// Handler for "/hall" that returns the current hall sensor value
void handleHallSensor() {
  hallValue = hall_sensor_read();  // Use hall_sensor_read() instead of hallRead()
  server.send(200, "text/plain", String(hallValue));
}

void setup() {
  Serial.begin(115200);

  // Initialize OLED display at I2C address 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 5);
  display.println("Connecting to WiFi...");
  display.display();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(".");
    attempt++;
    
    // Optionally, update OLED with attempt count
    display.clearDisplay();
    display.setCursor(10, 5);
    display.print("Attempt ");
    display.print(attempt);
    display.print(": ");
    display.println(WiFi.status());
    display.display();
  }
  
  display.clearDisplay();
  display.setCursor(10, 5);
  if (WiFi.status() == WL_CONNECTED) {
    display.println("WiFi Connected!");
    display.setCursor(10, 20);
    display.print("IP: ");
    display.println(WiFi.localIP());
    Serial.println("\nWiFi Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    display.println("WiFi Connection Failed!");
    Serial.println("\nWiFi Connection Failed!");
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/hall", handleHallSensor);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  
  // Read the hall sensor value using hall_sensor_read()
  hallValue = hall_sensor_read();
  
  // Update OLED display with hall sensor value
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 5);
  display.print("Hall:");
  display.setCursor(10, 20);
  display.print(hallValue);
  display.display();
  
  Serial.print("Hall Sensor Value: ");
  Serial.println(hallValue);
  
  delay(1000);
}
