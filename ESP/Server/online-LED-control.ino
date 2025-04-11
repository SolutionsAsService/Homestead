#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <pgmspace.h>  // for PROGMEM/FPSTR

// ——— Wi‑Fi credentials ———
const char* ssid     = "emeraldcity";
const char* password = "spacemonkeys";

// ——— OLED setup ———
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define OLED_ADDR     0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ——— LED pins & state ———
const int redLedPin   = 25;
const int greenLedPin = 26;
bool redState   = false;
bool greenState = false;

// ——— Synchronous web server ———
WebServer server(80);

// ——— HTML page stored in flash ———
static const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 LED Control</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 2em; }
    .button { padding: 16px 40px; margin: 8px; font-size: 24px; cursor: pointer; border: none; }
    .on  { background-color: #4CAF50; color: white; }
    .off { background-color: #f44336; color: white; }
  </style>
</head><body>
  <h1>ESP32 LED Control</h1>
  <p>Red LED: %RED%</p>
  <p>
    <a href="/red/on"><button class="button on">ON</button></a>
    <a href="/red/off"><button class="button off">OFF</button></a>
  </p>
  <p>Green LED: %GREEN%</p>
  <p>
    <a href="/green/on"><button class="button on">ON</button></a>
    <a href="/green/off"><button class="button off">OFF</button></a>
  </p>
</body></html>
)rawliteral";

// ——— Handlers ———
void handleRoot() {
  // Load template from flash, replace placeholders, send it
  String page = FPSTR(htmlPage);                      // FPSTR casts PROGMEM to FlashStringHelper :contentReference[oaicite:0]{index=0}
  page.replace("%RED%",   redState   ? "ON" : "OFF");
  page.replace("%GREEN%", greenState ? "ON" : "OFF");
  server.send(200, "text/html", page);
}

void handleRedOn() {
  redState = true;
  digitalWrite(redLedPin, HIGH);
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleRedOff() {
  redState = false;
  digitalWrite(redLedPin, LOW);
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleGreenOn() {
  greenState = true;
  digitalWrite(greenLedPin, HIGH);
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleGreenOff() {
  greenState = false;
  digitalWrite(greenLedPin, LOW);
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);

  // OLED init (I²C on 21=SDA, 22=SCL)
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed");
    while (true) delay(1000);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // LED pins (if >12 mA each, drive via MOSFET)
  pinMode(redLedPin,   OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  digitalWrite(redLedPin,   LOW);
  digitalWrite(greenLedPin, LOW);

  // Connect to Wi‑Fi
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Connecting WiFi...");
  display.display();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi Connected");
  display.print("IP: "); display.println(WiFi.localIP());
  display.display();

  // Route setup
  server.on("/",         HTTP_GET, handleRoot);
  server.on("/red/on",   HTTP_GET, handleRedOn);
  server.on("/red/off",  HTTP_GET, handleRedOff);
  server.on("/green/on", HTTP_GET, handleGreenOn);
  server.on("/green/off",HTTP_GET, handleGreenOff);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Keep Wi‑Fi alive
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
  }
  server.handleClient();  // process HTTP requests
}
