#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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
String redState   = "OFF";
String greenState = "OFF";

// ——— Web server on port 80 ———
WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  // Init OLED (I²C on 21=SDA, 22=SCL) :contentReference[oaicite:3]{index=3}
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed");
    while (true) delay(1000);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Show boot message
  display.setCursor(0,0);
  display.println("Booting...");
  display.display();

  // Setup LEDs
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  digitalWrite(redLedPin,   LOW);
  digitalWrite(greenLedPin, LOW);

  // Connect to Wi‑Fi
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }

  // Display Wi‑Fi result & IP
  display.clearDisplay();
  display.setCursor(0,0);
  if (WiFi.status() == WL_CONNECTED) {
    display.println("WiFi Connected");
    display.print("IP:");
    display.println(WiFi.localIP());
  } else {
    display.println("WiFi Failed!");
  }
  display.display();

  // Start server
  server.begin();
  Serial.println("Server started at " + WiFi.localIP().toString());
}

void loop() {
  // Reconnect if dropped
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(1000);
  }

  // Handle web clients
  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\r');
    client.flush();

    // Parse URL and set LEDs :contentReference[oaicite:4]{index=4}
    if (req.indexOf("GET /red/on")    >= 0) { digitalWrite(redLedPin, HIGH);   redState   = "ON";  }
    if (req.indexOf("GET /red/off")   >= 0) { digitalWrite(redLedPin, LOW);    redState   = "OFF"; }
    if (req.indexOf("GET /green/on")  >= 0) { digitalWrite(greenLedPin, HIGH); greenState = "ON";  }
    if (req.indexOf("GET /green/off") >= 0) { digitalWrite(greenLedPin, LOW);  greenState = "OFF"; }

    // Update OLED with current states
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("IP: " + WiFi.localIP().toString());
    display.print("Red: ");   display.println(redState);
    display.print("Green: "); display.println(greenState);
    display.display();

    // Send HTML page with buttons :contentReference[oaicite:5]{index=5}
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.print(R"rawliteral(
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
  <p>Red LED: )rawliteral");
    client.print(redState);
    client.print(R"rawliteral(</p>
  <p>
    <a href="/red/on"><button class="button on">ON</button></a>
    <a href="/red/off"><button class="button off">OFF</button></a>
  </p>
  <p>Green LED: )rawliteral");
    client.print(greenState);
    client.print(R"rawliteral(</p>
  <p>
    <a href="/green/on"><button class="button on">ON</button></a>
    <a href="/green/off"><button class="button off">OFF</button></a>
  </p>
</body></html>
)rawliteral");

    delay(1);
    client.stop();
  }
}
