#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// —— User Config —— //
const char* ssid      = "emeraldcity";
const char* password  = "spacemonkeys";
const char* city      = "Taos";
const char* country   = "US";
const char* owmApiKey = "29791c1b9f625367055f374a4260a2fc";
const char* ntpServer = "pool.ntp.org";
const char* timezone  = "MST7MDT";

// —— Intervals —— //
const unsigned long WEATHER_INTERVAL = 10UL * 60UL * 1000UL;
const unsigned long NTP_INTERVAL     = 30UL * 60UL * 1000UL;

// —— OLED config —— //
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_ADDR     0x3C
#define OLED_RESET    -1
#define SDA_PIN       21
#define SCL_PIN       22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// —— State —— //
unsigned long lastWeather = 0, lastNTP = 0;
float tempC = 0, tempF = 0;
int humidity = 0;
float windSpeed = 0;
String weatherDesc = "", iconCode = "";
int moonPhase = 0;

// —— Prototypes —— //
void connectWiFi();
bool syncTime();
void fetchWeather();
void drawDisplay();
int calcMoonPhase(int y, int m, int d);
void drawWeatherIcon(const String& ic, int x, int y);
void drawMoonIcon(int phase, int x, int y);

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed");
    while (1) delay(1000);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  connectWiFi();
  if (syncTime()) Serial.println("NTP OK");
  else Serial.println("NTP FAIL");

  fetchWeather();
}

void loop() {
  unsigned long now = millis();
  if (now - lastNTP > NTP_INTERVAL) syncTime();
  if (now - lastWeather > WEATHER_INTERVAL) fetchWeather();
  drawDisplay();
  delay(1000);
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Wi-Fi connecting");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    Serial.print('.');
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWi-Fi OK, IP=%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWi-Fi FAIL");
  }
}

bool syncTime() {
  configTzTime(timezone, ntpServer);
  struct tm tm;
  if (getLocalTime(&tm, 2000)) {
    lastNTP = millis();
    return true;
  }
  return false;
}

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  String url = String("http://api.openweathermap.org/data/2.5/weather?q=") +
               city + "," + country + "&units=metric&appid=" + owmApiKey;
  http.begin(url);
  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, http.getString());
    tempC       = doc["main"]["temp"].as<float>();
    tempF       = tempC * 9.0 / 5.0 + 32.0;
    humidity    = doc["main"]["humidity"].as<int>();
    windSpeed   = doc["wind"]["speed"].as<float>();
    weatherDesc = String((const char*)doc["weather"][0]["description"]).substring(0, 16);
    iconCode    = doc["weather"][0]["icon"].as<const char*>();
    Serial.println("Weather OK");
  } else {
    Serial.printf("Weather HTTP %d\n", code);
  }
  http.end();
  lastWeather = millis();
}

void drawDisplay() {
  display.clearDisplay();
  display.setCursor(2, 0);

  struct tm tm;
  char tbuf[16], dbuf[16];
  if (getLocalTime(&tm, 500)) {
    strftime(tbuf, sizeof(tbuf), "%I:%M %p", &tm);
    strftime(dbuf, sizeof(dbuf), "%m-%d-%Y", &tm);
    display.print(tbuf); display.print(" "); display.print(dbuf);
    moonPhase = calcMoonPhase(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
  } else {
    display.print("Time N/A");
  }

  // Line 2: Temperature (F) and Humidity
  display.setCursor(2, 8);
  display.printf("T:%.1fF  H:%d%%", tempF, humidity);
  drawWeatherIcon(iconCode, SCREEN_WIDTH - 12, 10); // moved icon 2px down

  // Line 3: Weather description
  display.setCursor(2, 16);
  display.print(weatherDesc);

  // Line 4: Wind speed and Moon Phase full name
  display.setCursor(2, 24);
  static const char* phasesFull[] = {
    "New Moon", "Wax Cres", "1st Qtr", "Wax Gib",
    "Full Moon", "Wan Gib", "Last Qtr", "Wan Cres"
  };
  display.printf("W:%.1fm/s %s", windSpeed, phasesFull[moonPhase]);
  drawMoonIcon(moonPhase, SCREEN_WIDTH - 12, 24);  // keep moon icon here

  display.display();
}



int calcMoonPhase(int Y, int M, int D) {
  if (M < 3) { Y--; M += 12; }
  ++M;
  double c = 365.25 * Y, e = 30.6 * M, jd = c + e + D - 694039.09;
  jd /= 29.5305882; int b = jd; jd -= b; b = jd * 8 + 0.5; b &= 7;
  return b;
}

void drawWeatherIcon(const String& ic, int x, int y) {
  if (ic.startsWith("01")) {
    display.fillCircle(x + 4, y + 4, 2, SSD1306_WHITE);
    const int rx[8] = {4, 7, 8, 7, 4, 1, 0, 1};
    const int ry[8] = {0, 1, 4, 7, 8, 7, 4, 1};
    for (int i = 0; i < 8; i++) {
      display.drawLine(x + 4, y + 4, x + rx[i], y + ry[i], SSD1306_WHITE);
    }
  }
  else if (ic.startsWith("02") || ic.startsWith("03") || ic.startsWith("04")) {
    display.fillCircle(x + 3, y + 5, 3, SSD1306_WHITE);
    display.fillCircle(x + 7, y + 5, 3, SSD1306_WHITE);
    display.fillRect(x + 3, y + 5, 7, 4, SSD1306_WHITE);
  }
  else if (ic.startsWith("09") || ic.startsWith("10")) {
    display.fillCircle(x + 3, y + 4, 3, SSD1306_WHITE);
    display.fillCircle(x + 7, y + 4, 3, SSD1306_WHITE);
    display.fillRect(x + 3, y + 4, 7, 4, SSD1306_WHITE);
    display.drawLine(x + 4, y + 9, x + 4, y + 11, SSD1306_WHITE);
    display.drawLine(x + 6, y + 9, x + 6, y + 11, SSD1306_WHITE);
    display.drawLine(x + 8, y + 9, x + 8, y + 11, SSD1306_WHITE);
  }
  else if (ic.startsWith("13")) {
    display.fillCircle(x + 3, y + 4, 3, SSD1306_WHITE);
    display.fillCircle(x + 7, y + 4, 3, SSD1306_WHITE);
    display.fillRect(x + 3, y + 4, 7, 4, SSD1306_WHITE);
    for (int dx = 4; dx <= 8; dx += 2) {
      display.drawPixel(x + dx, y + 9, SSD1306_WHITE);
      display.drawPixel(x + dx, y + 11, SSD1306_WHITE);
      display.drawPixel(x + dx - 1, y + 10, SSD1306_WHITE);
      display.drawPixel(x + dx + 1, y + 10, SSD1306_WHITE);
    }
  }
  else {
    display.fillCircle(x + 4, y + 5, 3, SSD1306_WHITE);
    display.fillCircle(x + 8, y + 5, 3, SSD1306_WHITE);
    display.fillRect(x + 4, y + 5, 7, 4, SSD1306_WHITE);
  }
}

void drawMoonIcon(int phase, int x, int y) {
  int cx = x + 4, cy = y + 4;
  display.drawCircle(cx, cy, 3, SSD1306_WHITE);

  switch (phase) {
    case 0: break;
    case 1:
      display.fillCircle(cx + 1, cy, 3, SSD1306_WHITE);
      display.fillCircle(cx - 1, cy, 3, SSD1306_BLACK);
      break;
    case 2:
      for (int dy = -3; dy <= 3; dy++) {
        int dx = sqrt(9 - dy * dy);
        display.drawFastHLine(cx, cy + dy, dx + 1, SSD1306_WHITE);
      }
      break;
    case 3:
      display.fillCircle(cx, cy, 3, SSD1306_WHITE);
      for (int dy = -3; dy <= 3; dy++) {
        int dx = sqrt(9 - dy * dy);
        display.drawFastHLine(cx - (dx / 2), cy + dy, dx / 2 + 1, SSD1306_BLACK);
      }
      break;
    case 4:
      display.fillCircle(cx, cy, 3, SSD1306_WHITE);
      break;
    case 5:
      display.fillCircle(cx, cy, 3, SSD1306_WHITE);
      for (int dy = -3; dy <= 3; dy++) {
        int dx = sqrt(9 - dy * dy);
        display.drawFastHLine(cx + (dx / 2), cy + dy, dx / 2 + 1, SSD1306_BLACK);
      }
      break;
    case 6:
      for (int dy = -3; dy <= 3; dy++) {
        int dx = sqrt(9 - dy * dy);
        display.drawFastHLine(cx - dx, cy + dy, dx + 1, SSD1306_WHITE);
      }
      break;
    case 7:
      display.fillCircle(cx - 1, cy, 3, SSD1306_WHITE);
      display.fillCircle(cx + 1, cy, 3, SSD1306_BLACK);
      break;
  }
}
