// Finance Clock with Finnhub API – Powered by Finnhub  
// Attribution required by Finnhub: https://finnhub.io

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// —— User Config —— //
const char* ssid       = "emeraldcity";
const char* password   = "spacemonkeys";
const char* city       = "Taos";
const char* country    = "US";
const char* owmKey     = "29791c1b9f625367055f374a4260a2fc";
const char* fhKey      = "d0029nhr01qud9qlca00d0029nhr01qud9qlca0g";
const char* ntpServer  = "pool.ntp.org";
const char* timezone   = "MST7MDT";  // Mountain Standard/Daylight Time

// —— Intervals —— //
const unsigned long WEATHER_INTERVAL = 10UL * 60UL * 1000UL;  // 10 min
const unsigned long NTP_INTERVAL     = 30UL * 60UL * 1000UL;  // 30 min
const unsigned long STOCK_INTERVAL   =  1UL * 60UL * 1000UL;  // 1 min

// —— OLED config —— //
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// —— State —— //
unsigned long lastWeather = 0, lastNTP = 0, lastStock = 0;
float   tempF = 0.0;
int     humidity = 0;
float   windSpeed = 0.0;
String  weatherDesc = "";
String  scrollText = "";
int16_t scrollX = 0, scrollW = 0;

// —— Six stock symbols —— //
const char* symbols[] = {
  "AAPL", "GOOGL", "MSFT", "AMZN", "TSLA", "NFLX"
};
const uint8_t symbolCount = sizeof(symbols) / sizeof(symbols[0]);

// ── Prototypes ───────────────────────────────────
void connectWiFi();
bool syncTime();
void fetchWeather();
void fetchStocks();
void drawDisplay();

void setup() {
  Serial.begin(115200);
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[ERROR] OLED init failed");
    while (true);
  }
  display.clearDisplay();

  connectWiFi();
  syncTime();            // initial NTP sync
  fetchWeather();        // immediate weather
  fetchStocks();         // immediate stocks
}

void loop() {
  unsigned long now = millis();
  if (now - lastNTP     > NTP_INTERVAL)     syncTime();
  if (now - lastWeather > WEATHER_INTERVAL) fetchWeather();
  if (now - lastStock   > STOCK_INTERVAL)   fetchStocks();
  drawDisplay();
  delay(100);
}

// ── Wi‑Fi & NTP ─────────────────────────────────
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("WiFi connecting \"%s\"…\n", ssid);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print('.');
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWiFi up, IP=%s\n",
      WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWiFi failed");
  }
}

bool syncTime() {
  Serial.println("Syncing NTP…");
  configTzTime(timezone, ntpServer);
  struct tm tm;
  int retries = 0;
  while (!getLocalTime(&tm) && retries < 3) {
    Serial.println("  retry NTP");
    delay(2000);
    retries++;
  }
  lastNTP = millis();
  return (retries < 3);
}

// ── Fetch Weather (Imperial) ─────────────────────
void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;
  String url = String("http://api.openweathermap.org/data/2.5/weather?q=")
             + city + "," + country
             + "&units=imperial&appid=" + owmKey;
  HTTPClient http; http.begin(url);
  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String body = http.getString();
    DynamicJsonDocument doc(2048);
    if (!deserializeJson(doc, body)) {
      tempF       = doc["main"]["temp"].as<float>();     // °F :contentReference[oaicite:9]{index=9}
      humidity    = doc["main"]["humidity"].as<int>();   // % :contentReference[oaicite:10]{index=10}
      windSpeed   = doc["wind"]["speed"].as<float>();    // m/s :contentReference[oaicite:11]{index=11}
      weatherDesc = String((const char*)doc["weather"][0]["description"])
                     .substring(0, 12);                  // desc :contentReference[oaicite:12]{index=12}
      Serial.println("Weather updated");
    }
  } else {
    Serial.printf("[HTTP] Weather err: %d\n", code);
  }
  http.end();
  lastWeather = millis();
}

// ── Fetch Stocks & Prepare Scroll ────────────────
void fetchStocks() {
  if (WiFi.status() != WL_CONNECTED) return;
  scrollText = "";
  for (uint8_t i = 0; i < symbolCount; i++) {
    const char* s = symbols[i];
    String url = String("https://finnhub.io/api/v1/quote?symbol=")
               + s + "&token=" + fhKey;                // Finnhub quote :contentReference[oaicite:13]{index=13}
    HTTPClient http; http.begin(url);
    int code = http.GET();
    if (code == HTTP_CODE_OK) {
      String body = http.getString();
      DynamicJsonDocument doc(512);
      if (!deserializeJson(doc, body)) {
        float price = doc["c"].as<float>();             // current price :contentReference[oaicite:14]{index=14}
        scrollText += String(s) + ":$" + String(price,2) + "   ";
      }
    } else {
      Serial.printf("[HTTP] Stock err %d for %s\n", code, s);
    }
    http.end();
    delay(150);  // ≤60 calls/min :contentReference[oaicite:15]{index=15}
  }
  // compute scroll width
  int16_t x1, y1; uint16_t w, h;
  display.setTextSize(1);
  display.getTextBounds(scrollText, 0, 0, &x1, &y1, &w, &h);
  scrollW = w; scrollX = SCREEN_WIDTH;
  lastStock = millis();
}

// ── Draw All Lines ───────────────────────────────
void drawDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // Line 1: Time & Date
  struct tm tm; char buf1[20];
  if (getLocalTime(&tm)) {
    strftime(buf1, sizeof(buf1), "%I:%M %p %m-%d-%y", &tm);
  } else {
    strcpy(buf1, "Time N/A");
  }
  display.setCursor(0, 0);
  display.print(buf1);

  // Line 2: Temp °F + Weather
  display.setCursor(0, 8);
  display.printf("T:%.0f°F %s", tempF, weatherDesc.c_str());

  // Line 3: Humidity & Wind
  display.setCursor(0, 16);
  display.printf("H:%d%% W:%.1f m/s", humidity, windSpeed);

  // Line 4: Scrolling Finance
  display.setCursor(scrollX, 24);
  display.print(scrollText);
  if (--scrollX < -scrollW) scrollX = SCREEN_WIDTH;

  display.display();
}
