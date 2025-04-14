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
const char* owmApiKey  = "29791c1b9f625367055f374a4260a2fc";
const char* ntpServer  = "pool.ntp.org";
const char* timezone   = "MST7MDT"; // POSIX TZ string for Mountain Standard/Daylight Time

// —— Intervals —— //
const unsigned long WEATHER_INTERVAL = 10UL * 60UL * 1000UL; // 10 min
const unsigned long NTP_INTERVAL     = 30UL * 60UL * 1000UL; // 30 min

// —— OLED config —— //
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_ADDR     0x3C
#define OLED_RESET    -1
#define SDA_PIN       21
#define SCL_PIN       22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// —— State —— //
unsigned long lastWeather = 0;
unsigned long lastNTP     = 0;

float   tempC       = 0.0;
int     humidity    = 0;
String  weatherDesc = "";
String  iconCode    = "";

// —— Prototypes —— //
void connectWiFi();
void ensureWiFi();
bool syncTimeBlocking();
void fetchWeather();
void retryWeatherFetch();
void retryTimeSync();
void drawDisplay();
void drawWeatherIcon(const String& iconCode, int x, int y);

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("ERROR: SSD1306 init failed");
    while (1) delay(1000);
  }
  display.clearDisplay();
  display.display();

  connectWiFi();

  if (!syncTimeBlocking()) {
    Serial.println("Error: initial NTP sync failed");
  }

  retryWeatherFetch(); // get weather right away
}

void loop() {
  unsigned long now = millis();

  ensureWiFi();

  if (now - lastNTP > NTP_INTERVAL) {
    retryTimeSync();
  }
  if (now - lastWeather > WEATHER_INTERVAL) {
    retryWeatherFetch();
  }

  drawDisplay();
  delay(1000);
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to Wi‑Fi \"%s\"…\n", ssid);
  
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    Serial.print('.');
    delay(500);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nConnected, IP=%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWi‑Fi failed; will retry in loop");
  }
}

void ensureWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi‑Fi lost—reconnecting");
    WiFi.disconnect();
    connectWiFi();
  }
}

bool syncTimeBlocking() {
  Serial.println("Syncing NTP time…");
  configTzTime(timezone, ntpServer);
  
  struct tm tm;
  int retries = 0;
  while (!getLocalTime(&tm, 10000) && retries < 3) {
    Serial.printf("  retrying NTP (%d/3)…\n", retries+1);
    delay(2000);
    retries++;
  }
  if (retries == 3) {
    Serial.println("NTP sync failed");
    return false;
  }
  Serial.printf("Time now: %04d-%02d-%02d %02d:%02d:%02d\n",
                tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec);
  lastNTP = millis();
  return true;
}

void retryTimeSync() {
  if (!syncTimeBlocking()) {
    Serial.println("Periodic NTP sync failed");
  }
}

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No Wi‑Fi—skipping weather");
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
    if (!deserializeJson(doc, body)) {
      tempC       = doc["main"]["temp"].as<float>();
      humidity    = doc["main"]["humidity"].as<int>();
      weatherDesc = String((const char*)doc["weather"][0]["description"])
                       .substring(0, 16);
      iconCode    = String((const char*)doc["weather"][0]["icon"]);
      Serial.println("Weather updated");
    } else {
      Serial.println("JSON parse error");
    }
  } else {
    Serial.printf("HTTP error: %d\n", code);
  }
  http.end();
  lastWeather = millis();
}

void retryWeatherFetch() {
  int retries = 0;
  while (retries < 3 && lastWeather == 0) {
    fetchWeather();
    if (lastWeather != 0) break;
    Serial.printf("  retrying weather (%d/3)…\n", retries+1);
    delay(2000);
    retries++;
  }
  if (retries == 3) {
    Serial.println("Weather fetch failed");
  }
}

void drawDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  struct tm tm;
  char timeBuf[16], dateBuf[16];
  if (getLocalTime(&tm, 1000)) {
    strftime(timeBuf, sizeof(timeBuf), "%H:%M", &tm);
    strftime(dateBuf, sizeof(dateBuf), "%m-%d-%Y", &tm);
    // —— Line 1: time + date, with 2px top margin
    display.setCursor(2, 2);
    display.print(timeBuf);
    display.print(" ");
    display.print(dateBuf);
  } else {
    display.setCursor(2, 2);
    display.print("Time N/A");
  }

  // —— Line 2: temp & humidity
  display.setCursor(2, 12);
  display.printf("T:%.1fC  H:%d%%", tempC, humidity);

  // —— Line 3: weather description + icon
  display.setCursor(2, 22);
  display.print(weatherDesc);
  drawWeatherIcon(iconCode, SCREEN_WIDTH - 12, 22);

  display.display();
}

// Draw a tiny 8×8 icon for clear/cloudy/rainy
void drawWeatherIcon(const String& ic, int x, int y) {
  if (ic.startsWith("01")) {
    // sun
    display.drawCircle(x+4, y+4, 3, SSD1306_WHITE);
    display.drawLine(x+4, y+0, x+4, y+2, SSD1306_WHITE);
    display.drawLine(x+4, y+6, x+4, y+8, SSD1306_WHITE);
    display.drawLine(x+0, y+4, x+2, y+4, SSD1306_WHITE);
    display.drawLine(x+6, y+4, x+8, y+4, SSD1306_WHITE);
  }
  else if (ic.startsWith("02") || ic.startsWith("03") || ic.startsWith("04")) {
    // cloud
    display.fillCircle(x+3, y+4, 3, SSD1306_WHITE);
    display.fillCircle(x+6, y+4, 3, SSD1306_WHITE);
    display.fillRect  (x+3, y+4, 6, 4, SSD1306_WHITE);
  }
  else if (ic.startsWith("09") || ic.startsWith("10")) {
    // rain
    display.fillCircle(x+3, y+4, 3, SSD1306_WHITE);
    display.fillCircle(x+6, y+4, 3, SSD1306_WHITE);
    display.fillRect  (x+3, y+4, 6, 4, SSD1306_WHITE);
    display.drawLine  (x+4, y+9, x+4, y+11, SSD1306_WHITE);
    display.drawLine  (x+6, y+9, x+6, y+11, SSD1306_WHITE);
  }
  else {
    // default: little cloud
    display.fillCircle(x+3, y+4, 3, SSD1306_WHITE);
    display.fillCircle(x+6, y+4, 3, SSD1306_WHITE);
    display.fillRect  (x+3, y+4, 6, 4, SSD1306_WHITE);
  }
}
