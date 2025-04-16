#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ── Credentials & API ────────────────────────────────────────────────
const char* ssid       = "emeraldcity";
const char* password   = "spacemonkeys";
const char* apiKey     = "YOUR AV API KEY";
const char* apiBaseURL = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=";

// ── Tickers ──────────────────────────────────────────────────────────
const char* symbols[] = { "AAPL", "GOOGL", "MSFT", "AMZN" };
const uint8_t symbolCount = sizeof(symbols) / sizeof(symbols[0]);

// ── OLED Setup ───────────────────────────────────────────────────────
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ── Timing & State ───────────────────────────────────────────────────
volatile bool wifiReady = false;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 60000; // 1 minute
uint8_t nextSymbolIndex = 0;

// ── Forward Declarations ────────────────────────────────────────────
void onGotIP          (WiFiEvent_t event, WiFiEventInfo_t info);
void onDisconnected   (WiFiEvent_t event, WiFiEventInfo_t info);
void initWiFi();
void fetchAndDisplay();

void setup() {
  Serial.begin(115200);
  delay(500);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[ERROR] OLED init failed!");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Starting...");
  display.display();

  // Wi‑Fi & events
  initWiFi();
}

void loop() {
  unsigned long now = millis();
  if (wifiReady && (now - lastUpdate >= updateInterval)) {
    lastUpdate = now;
    fetchAndDisplay();
    nextSymbolIndex = (nextSymbolIndex + 1) % symbolCount;
  }
}

// ── Wi‑Fi Initialization ─────────────────────────────────────────────
void initWiFi() {
  // Clean up any old state
  WiFi.disconnect(true, true);
  wifiReady = false;

  // Register event handlers
  WiFi.onEvent(onGotIP,        ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(onDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  // Set up and connect
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  Serial.printf("[WiFi] Connecting to %s\n", ssid);
}

// ── Event: Got IP ────────────────────────────────────────────────────
void onGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.print("[WiFi] CONNECTED, IP=");
  Serial.println(WiFi.localIP());
  wifiReady = true;
  // Reset timer so we fetch immediately
  lastUpdate = 0;
}

// ── Event: Disconnected ──────────────────────────────────────────────
void onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.printf(
    "[WiFi] DISCONNECTED, reason=%d\n",
    info.wifi_sta_disconnected.reason
  );
  wifiReady = false;
  // Try to reconnect in background
  WiFi.reconnect();
}

// ── Fetch Quotes & Update OLED ──────────────────────────────────────
void fetchAndDisplay() {
  display.clearDisplay();

  for (uint8_t line = 0; line < symbolCount; line++) {
    const char* sym = symbols[(nextSymbolIndex + line) % symbolCount];
    String url = String(apiBaseURL) + sym + "&apikey=" + apiKey;
    Serial.println("[HTTP] GET " + url);

    HTTPClient http;
    http.begin(url);
    int code = http.GET();
    String text = "---";

    if (code == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("[HTTP] Payload: " + payload);
      DynamicJsonDocument doc(JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(8) + 200);
      if (deserializeJson(doc, payload) == DeserializationError::Ok) {
        text = doc["Global Quote"]["05. price"] | "---";
      } else {
        Serial.println("[JSON] parse error");
      }
    } else {
      Serial.printf("[HTTP] err %d\n", code);
    }
    http.end();

    display.setCursor(0, line * 8);
    display.printf("%s: $%s", sym, text.c_str());
    delay(200);  // respect rate limits
  }

  display.display();
}
