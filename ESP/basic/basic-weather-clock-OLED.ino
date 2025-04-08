#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// —— User Config —— //
const char* ssid       = "emeraldcity";
const char* password   = "spacemonkeys";
const char* city       = "Taos";
const char* country    = "US";
const char* owmApiKey  = "API KEY";
const char* ntpServer  = "pool.ntp.org";
const char* timezone   = "MST7MDT";  // POSIX TZ for MST/MDT

// —— Intervals —— //
const unsigned long WEATHER_INTERVAL = 10UL * 60UL * 1000UL;  // 10 min
const unsigned long NTP_INTERVAL     = 30UL * 60UL * 1000UL;  // 30 min

// —— OLED config —— //
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 32
#define OLED_ADDR     0x3C
#define OLED_RESET    -1
#define SDA_PIN       21
#define SCL_PIN       22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// —— State —— //
unsigned long lastWeather = 0;
unsigned long lastNTP     = 0;

float  tempC       = 0.0;
float  tempF       = 0.0;
int    humidity    = 0;
String weatherDesc = "";

// —— Prototypes —— //
void  connectWiFi();
void  ensureWiFi();
bool  syncTime();
void  fetchWeather();
void  drawDisplay();

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("ERROR: SSD1306 failed");
    while (1) delay(1000);
  }
  display.clearDisplay();
  display.display();

  connectWiFi();
  syncTime();
  fetchWeather();
}

void loop() {
  unsigned long now = millis();

  ensureWiFi();

  if (now - lastNTP > NTP_INTERVAL) {
    syncTime();
  }
  if (now - lastWeather > WEATHER_INTERVAL) {
    fetchWeather();
  }

  drawDisplay();
  delay(1000);
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Wi‑Fi connecting to \"%s\" …\n", ssid);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    Serial.print('.');
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nConnected, IP=%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWi‑Fi FAILED");
  }
}

void ensureWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi‑Fi lost — reconnecting");
    WiFi.disconnect();
    connectWiFi();
  }
}

bool syncTime() {
  Serial.println("Syncing NTP time…");
  // set POSIX TZ & start NTP
  setenv("TZ", timezone, 1);
  tzset();
  configTime(0, 0, ntpServer);

  struct tm tm;
  if (getLocalTime(&tm, 10000)) {
    Serial.printf("Time: %04d-%02d-%02d %02d:%02d:%02d\n",
      tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
      tm.tm_hour, tm.tm_min, tm.tm_sec
    );
    lastNTP = millis();
    return true;
  } else {
    Serial.println("Failed to get time");
    return false;
  }
}

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No Wi‑Fi — skipping weather");
    return;
  }

  String url = String("http://api.openweathermap.org/data/2.5/weather?q=")
             + city + "," + country
             + "&units=metric&appid=" + owmApiKey;

  HTTPClient http;
  http.begin(url);
  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String body = http.getString();
    DynamicJsonDocument doc(4096);
    auto err = deserializeJson(doc, body);
    if (!err) {
      tempC       = doc["main"]["temp"].as<float>();
      tempF       = tempC * 9.0/5.0 + 32.0;
      humidity    = doc["main"]["humidity"].as<int>();
      weatherDesc = doc["weather"][0]["description"].as<const char*>();
      if (weatherDesc.length() > 16)
        weatherDesc = weatherDesc.substring(0,16);
      Serial.println("Weather OK");
    } else {
      Serial.print("JSON error: "); Serial.println(err.c_str());
    }
  } else {
    Serial.printf("HTTP error: %d\n", code);
  }
  http.end();
  lastWeather = millis();
}

void drawDisplay() {
  display.clearDisplay();
  display.drawRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  struct tm tm;
  if (getLocalTime(&tm, 1000)) {
    char tbuf[9], dbuf[11];
    strftime(tbuf, sizeof(tbuf), "%H:%M:%S", &tm);
    strftime(dbuf, sizeof(dbuf), "%Y-%m-%d", &tm);
    display.setCursor(2, 0);  display.print(tbuf);
    display.setCursor(2, 8);  display.print(dbuf);
  } else {
    display.setCursor(2, 0);
    display.print("Time N/A");
  }

  display.setCursor(2, 16);
  display.printf("T:%.1fC/%.1fF", tempC, tempF);

  display.setCursor(2, 24);
  display.printf("H:%d%% %s", humidity, weatherDesc.c_str());

  display.display();
}
