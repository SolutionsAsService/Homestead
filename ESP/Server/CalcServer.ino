#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi credentials - update these
const char* ssid = "emeraldcity";
const char* password = "spacemonkeys";

// OLED display settings for a 128x32 display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C
#define OLED_SDA      21
#define OLED_SCL      22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create a web server on port 80
WebServer server(80);

// HTML for the calculator webpage with basic CSS styling for a professional look
String calculatorPage() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32 Calculator</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; background-color: #f2f2f2; text-align: center; }";
  html += "h1 { color: #333; }";
  html += "form { background-color: #fff; padding: 20px; margin: auto; width: 300px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
  html += "input[type='text'], select { padding: 8px; margin: 5px; width: 90%; border: 1px solid #ccc; border-radius: 4px; }";
  html += "input[type='submit'] { padding: 10px 20px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer; }";
  html += "input[type='submit']:hover { background-color: #45a049; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>ESP32 Calculator</h1>";
  html += "<form action='/calculate' method='get'>";
  html += "Number 1: <input type='text' name='num1' required><br>";
  html += "Operator: <select name='op'>";
  html += "<option value='+'>+</option>";
  html += "<option value='-'>-</option>";
  html += "<option value='*'>*</option>";
  html += "<option value='/'>/</option>";
  html += "</select><br>";
  html += "Number 2: <input type='text' name='num2' required><br>";
  html += "<input type='submit' value='Calculate'>";
  html += "</form>";
  html += "</body></html>";
  return html;
}

// Serve the calculator page
void handleRoot() {
  server.send(200, "text/html", calculatorPage());
}

// Handle calculation requests (e.g., /calculate?num1=12&op=*&num2=3)
void handleCalculate() {
  if(!server.hasArg("num1") || !server.hasArg("num2") || !server.hasArg("op")){
    server.send(400, "text/plain", "Missing parameter");
    return;
  }
  
  float num1 = server.arg("num1").toFloat();
  float num2 = server.arg("num2").toFloat();
  String op = server.arg("op");
  float result = 0;
  
  if(op == "+") {
    result = num1 + num2;
  } else if(op == "-") {
    result = num1 - num2;
  } else if(op == "*") {
    result = num1 * num2;
  } else if(op == "/") {
    if(num2 == 0) {
      server.send(400, "text/plain", "Division by zero error");
      return;
    }
    result = num1 / num2;
  } else {
    server.send(400, "text/plain", "Invalid operator");
    return;
  }
  
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Calculation Result</title>";
  html += "<style>body { font-family: Arial, sans-serif; background-color: #f2f2f2; text-align: center; }";
  html += "h1 { color: #333; }";
  html += "div { background-color: #fff; padding: 20px; margin: auto; width: 300px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
  html += "a { text-decoration: none; color: #4CAF50; }";
  html += "</style></head><body>";
  html += "<h1>Calculation Result</h1><div>";
  html += String(num1) + " " + op + " " + String(num2) + " = " + String(result);
  html += "<br><br><a href='/'>Back to Calculator</a>";
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  
  // Initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)){
    Serial.println("SSD1306 allocation failed");
    while(1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting to WiFi...");
  display.display();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Display project title and hosting IP on OLED
  display.clearDisplay();
  display.setCursor(2, 2);
  display.println("ESP32 Calculator");
  display.println(WiFi.localIP());
  display.display();
  
  // Set up web server routes
  server.on("/", handleRoot);
  server.on("/calculate", handleCalculate);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
