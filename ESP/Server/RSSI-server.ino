#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ——— Your Wi‑Fi credentials here ———
const char* ssid     = "emeraldcity";
const char* password = "spacemonkeys";

// ——— OLED settings ———
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1       // not used
#define OLED_ADDR     0x3C     // 0x3C for 128×32 SSD1306
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ——— Web server on port 80 ———
WiFiServer server(80);

// ——— Simple HTML page ———
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ESP32 Status</title>
  <style>
    body { font-family: sans-serif; text-align:center; margin-top:2em; }
    h1 { color: #0077cc; }
    p { font-size:1.1em; }
  </style>
</head>
<body>
  <h1>ESP32 Web Server</h1>
  <p>Status: <span id="status">Online</span></p>
  <p>IP: <span id="ip">...</span></p>
  <p>RSSI: <span id="rssi">...</span> dBm</p>
  <script>
    function update() {
      fetch('/status')
        .then(r => r.json())
        .then(j => {
          document.getElementById('ip').textContent = j.ip;
          document.getElementById('rssi').textContent = j.rssi;
        })
        .catch(console.error);
    }
    setInterval(update, 2000);
    update();
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  // Init OLED
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {  // SSD1306_SWITCHCAPVCC, address 0x3C :contentReference[oaicite:2]{index=2}
    Serial.println("OLED init failed");
    while (true) delay(1000);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Connect to Wi‑Fi
  display.setCursor(0,0);
  display.println("Connecting to Wi-Fi...");
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi Failed!");
    display.display();
    // Let loop() handle reconnects
  } else {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi Connected");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.display();
  }

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Reconnect Wi‑Fi if dropped
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(1000);
  }

  // Update OLED every second
  static unsigned long lastOled = 0;
  if (millis() - lastOled > 1000) {
    lastOled = millis();
    display.clearDisplay();
    display.setCursor(0,0);
    if (WiFi.status() == WL_CONNECTED) {
      display.println("Status: Online");
      display.print("IP: ");
      display.println(WiFi.localIP());
      display.print("RSSI:");
      display.println(WiFi.RSSI());    // real‑time RSSI :contentReference[oaicite:3]{index=3}
    } else {
      display.println("Status: Offline");
      display.println("Reconnecting...");
    }
    display.display();
  }

  // Handle HTTP clients
  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\r');
    client.flush();

    // Serve JSON status
    if (req.indexOf("GET /status") >= 0) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.printf("{\"ip\":\"%s\",\"rssi\":%d}", WiFi.localIP().toString().c_str(), WiFi.RSSI());
    }
    // Serve main page
    else {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.print(htmlPage);
    }
    delay(1);
    client.stop();
  }
}
