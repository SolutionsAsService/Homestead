#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi credentials
const char* ssid = "";
const char* password = "";

// OLED display settings (128x32)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C
#define OLED_SDA      21
#define OLED_SCL      22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Create a web server on port 80
WebServer server(80);

// Serve the calculator page
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP32 Calculator</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; background-color: #f2f2f2; margin: 0; padding: 20px; }";
  html += "h1 { color: #333; text-align: center; }";
  html += "form { background-color: #fff; padding: 20px; margin: auto; width: 90%; max-width: 400px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
  html += "input[type='text'], select { padding: 10px; margin: 10px 0; width: 100%; border: 1px solid #ccc; border-radius: 4px; }";
  html += "input[type='submit'] { padding: 10px; width: 100%; background-color: #4CAF50; border: none; color: white; font-size: 16px; border-radius: 4px; cursor: pointer; }";
  html += "input[type='submit']:hover { background-color: #45a049; }";
  html += "#result { text-align: center; font-size: 18px; color: #333; margin-top: 20px; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>ESP32 Calculator</h1>";
  html += "<form id='calcForm'>";
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
  html += "<div id='result'></div>";
  html += "<script>";
  html += "document.getElementById('calcForm').addEventListener('submit', function(e){";
  html += "  e.preventDefault();";
  html += "  var formData = new FormData(this);";
  html += "  var num1 = formData.get('num1');";
  html += "  var op = formData.get('op');";
  html += "  var num2 = formData.get('num2');";
  html += "  fetch('/calculate?num1=' + encodeURIComponent(num1) + '&op=' + encodeURIComponent(op) + '&num2=' + encodeURIComponent(num2))";
  html += "    .then(response => response.json())";
  html += "    .then(data => {";
  html += "       document.getElementById('result').innerHTML = '<p>' + data.equation + ' = ' + data.result + '</p>';";
  html += "    })";
  html += "    .catch(err => {";
  html += "       document.getElementById('result').innerHTML = '<p>Error: ' + err + '</p>';";
  html += "    });";
  html += "});";
  html += "</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Handle calculation request and return JSON
void handleCalculate() {
  if(!server.hasArg("num1") || !server.hasArg("num2") || !server.hasArg("op")){
    server.send(400, "application/json", "{\"error\":\"Missing parameter\"}");
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
      server.send(400, "application/json", "{\"error\":\"Division by zero\"}");
      return;
    }
    result = num1 / num2;
  } else {
    server.send(400, "application/json", "{\"error\":\"Invalid operator\"}");
    return;
  }
  
  String equation = String(num1) + " " + op + " " + String(num2);
  String json = "{\"equation\":\"" + equation + "\",\"result\":" + String(result) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  
  // Initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)){
    Serial.println("OLED allocation failed");
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
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Display title and hosting IP on OLED
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
