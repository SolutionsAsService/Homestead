#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "emeraldcity";
const char* password = "spacemonkeys";

WebServer server(80);

// OLED settings
#define OLED_ADDR     0x3C
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// HTML page for the calculator app
String generateCalculatorHtml(String result = "") {
  String html = R"rawliteral(
    <!DOCTYPE HTML>
    <html>
      <head>
        <title>ESP Calculator</title>
        <style>
          body { font-family: Arial, sans-serif; max-width: 600px; margin: auto; padding: 20px; }
          .calculator {
            background-color: #f0f0f0;
            padding: 20px;
            border-radius: 10px;
          }
          input[type="text"] { width: 200px; }
          button { margin: 5px; }
        </style>
      </head>
      <body>
        <div class="calculator">
          <h1>ESP Calculator</h1>
          <form action="/calculate" method="POST">
            <input type="text" name="num1" placeholder="Number 1"><br><br>
            <select name="op">
              <option value="+">+</option>
              <option value="-">-</option>
              <option value="*">×</option>
              <option value="/">÷</option>
            </select><br><br>
            <input type="text" name="num2" placeholder="Number 2"><br><br>
            <button type="submit">Calculate</button>
          </form>
          <p id="result">)" + result + R"rawliteral(</p>
        </div>
      </body>
    </html>
  )rawliteral";
  return html;
}

String calculate(String num1Str, String op, String num2Str) {
  double num1 = atof(num1Str.c_str());
  double num2 = atof(num2Str.c_str());

  switch(op[0]) {
    case '+': return String(num1 + num2);
    case '-': return String(num1 - num2);
    case '*': return String(num1 * num2);
    case '/': 
      if (num2 != 0) return String(num1 / num2);
      return "Division by zero!";
    default: return "Invalid operator!";
  }
}

void handleRoot() {
  String html = generateCalculatorHtml();
  server.send(200, "text/html", html);
}

void handleCalculate() {
  if (server.method() == HTTP_POST) {
    if (server.hasArg("num1") && server.hasArg("op") && server.hasArg("num2")) {
      String num1 = server.arg("num1");
      String op = server.arg("op");
      String num2 = server.arg("num2");
      String result = calculate(num1, op, num2);
      String html = generateCalculatorHtml(result);
      server.send(200, "text/html", html);
    } else {
      server.send(400, "text/plain", "Missing parameters");
    }
  } else {
    server.send(405, "text/plain", "Method not allowed");
  }
}

void setupDisplay() {
  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 allocation failed");
    for(;;); // Loop forever if display initialization fails
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("ESP Calculator");
  display.display();
}

void updateOled(String message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Device Status:");
  display.println(message);
  display.display();
}

void connectToWiFi() {
  Serial.println("Attempting to connect to Wi-Fi...");
  updateOled("Connecting WiFi...");
  
  WiFi.disconnect(true);  // clear old connections
  WiFi.begin(ssid, password);
  
  unsigned long startAttemptTime = millis();
  const unsigned long connectionTimeout = 30000; // 30 seconds timeout
  
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < connectionTimeout) {
    delay(500);
    Serial.print(".");
    // Update OLED with elapsed time for debugging
    updateOled("Connecting: " + String((millis() - startAttemptTime) / 1000) + " sec");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWi-Fi connection failed with status: " + String(WiFi.status()));
    updateOled("WiFi failed (" + String(WiFi.status()) + ")");
    // Wait and then retry connecting
    delay(5000);
    connectToWiFi();
  } else {
    Serial.println("\nConnected to Wi-Fi!");
    String ipAddress = WiFi.localIP().toString();
    Serial.println("Local IP: " + ipAddress);
    updateOled("IP: " + ipAddress);
  }
}

void setup() {
  Serial.begin(115200);
  setupDisplay();
  connectToWiFi();

  // Set up web server endpoints
  server.on("/", handleRoot);
  server.on("/calculate", handleCalculate);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  // Check if Wi-Fi connection is still active; if not, reconnect
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi lost. Reconnecting...");
    updateOled("WiFi lost! Reconnecting...");
    connectToWiFi();
  }
  server.handleClient();
}
