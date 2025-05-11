#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// —— User Config —— //
const char* ssid      = "emeraldcity";
const char* password  = "spacemonkeys";
const char* city      = "Taos";
const char* country   = "US";
const char* owmApiKey = "YOUR WEATHER API KEY";

// —— Intervals —— //
const unsigned long WEATHER_INTERVAL = 10UL * 60UL * 1000UL;

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
float currTempC, currTempF;
int   currHumidity;
String currDesc, currIcon;

float nextTempC, nextTempF;
String nextDesc, nextIcon;

float todayHighC, todayLowC;
float todayHighF, todayLowF;

// —— Prototypes —— //
void connectWiFi();
void fetchWeather();
void drawDisplay();
void drawWeatherIcon(const String& ic, int x, int y);

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
  fetchWeather();
}

void loop() {
  if (millis() - lastWeather > WEATHER_INTERVAL) {
    fetchWeather();
  }
  drawDisplay();
  delay(1000);
}

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  Serial.print("Wi-Fi connecting");
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    Serial.print('.');
    delay(500);
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Wi-Fi OK, IP=%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("Wi-Fi FAIL");
  }
}

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED) return;
  }

  HTTPClient http;

  // Current weather
  {
    String url = String("http://api.openweathermap.org/data/2.5/weather?q=")
               + city + "," + country
               + "&units=metric&appid=" + owmApiKey;
    http.begin(url);
    if (http.GET() == HTTP_CODE_OK) {
      DynamicJsonDocument doc(1024);
      if (!deserializeJson(doc, http.getString())) {
        currTempC    = doc["main"]["temp"].as<float>();
        currTempF    = currTempC * 9.0 / 5.0 + 32.0;
        currHumidity = doc["main"]["humidity"].as<int>();
        currDesc     = String((const char*)doc["weather"][0]["description"]).substring(0,16);
        currIcon     = String((const char*)doc["weather"][0]["icon"]);
      }
    }
    http.end();
  }

  // 3-hour forecast (first entry)
  {
    String url = String("http://api.openweathermap.org/data/2.5/forecast?q=")
               + city + "," + country
               + "&units=metric&cnt=2&appid=" + owmApiKey;
    http.begin(url);
    if (http.GET() == HTTP_CODE_OK) {
      DynamicJsonDocument doc(2048);
      if (!deserializeJson(doc, http.getString())) {
        JsonVariant e = doc["list"][1];
        nextTempC = e["main"]["temp"].as<float>();
        nextTempF = nextTempC * 9.0 / 5.0 + 32.0;
        nextDesc  = String((const char*)e["weather"][0]["description"]).substring(0,16);
        nextIcon  = String((const char*)e["weather"][0]["icon"]);

        todayHighC = doc["list"][0]["main"]["temp_max"].as<float>();
        todayLowC  = doc["list"][0]["main"]["temp_min"].as<float>();
        todayHighF = todayHighC * 9.0 / 5.0 + 32.0;
        todayLowF  = todayLowC  * 9.0 / 5.0 + 32.0;
      }
    }
    http.end();
  }

  lastWeather = millis();
}

void drawDisplay() {
  display.clearDisplay();

  // Top right: weather icons
  drawWeatherIcon(currIcon, 100, 0);   // Current icon (top-right)
  drawWeatherIcon(nextIcon, 112, 0);   // Next forecast icon

  // Line 1: Current temp and humidity
  display.setCursor(0, 0);
  display.printf("Now:%3.0fF H:%2d%%", currTempF, currHumidity);

  // Line 2: Current condition
  display.setCursor(0, 8);
  display.print(currDesc);

  // Line 3: High and Low today
  display.setCursor(0, 16);
  display.printf("H:%2.0fF L:%2.0fF", todayHighF, todayLowF);

  // Line 4: Forecast in 3h
  display.setCursor(0, 24);
  display.printf("3h:%3.0fF ", nextTempF);
  display.print(nextDesc);

  display.display();
}

void drawWeatherIcon(const String& ic, int x, int y) {
  if (ic.startsWith("01")) {
    // sun
    display.fillCircle(x+4, y+4, 2, SSD1306_WHITE);
    const int rx[8] = {4,7,8,7,4,1,0,1}, ry[8] = {0,1,4,7,8,7,4,1};
    for (int i=0; i<8; i++) display.drawLine(x+4, y+4, x+rx[i], y+ry[i], SSD1306_WHITE);
  }
  else if (ic.startsWith("0")) {
    // clouds
    display.fillCircle(x+3, y+5, 3, SSD1306_WHITE);
    display.fillCircle(x+7, y+5, 3, SSD1306_WHITE);
    display.fillRect(x+3, y+5, 7, 4, SSD1306_WHITE);
  }
  else if (ic.startsWith("09") || ic.startsWith("10")) {
    // rain
    display.fillCircle(x+3, y+4, 3, SSD1306_WHITE);
    display.fillCircle(x+7, y+4, 3, SSD1306_WHITE);
    display.fillRect(x+3, y+4, 7, 4, SSD1306_WHITE);
    display.drawLine(x+4, y+9, x+4, y+11, SSD1306_WHITE);
    display.drawLine(x+6, y+9, x+6, y+11, SSD1306_WHITE);
    display.drawLine(x+8, y+9, x+8, y+11, SSD1306_WHITE);
  }
  else if (ic.startsWith("13")) {
    // snow
    display.fillCircle(x+3, y+4, 3, SSD1306_WHITE);
    display.fillCircle(x+7, y+4, 3, SSD1306_WHITE);
    display.fillRect(x+3, y+4, 7, 4, SSD1306_WHITE);
    for (int dx=4; dx<=8; dx+=2) display.drawPixel(x+dx, y+10, SSD1306_WHITE);
  }
  else {
    // fallback cloud
    display.fillCircle(x+4, y+5, 3, SSD1306_WHITE);
    display.fillCircle(x+8, y+5, 3, SSD1306_WHITE);
    display.fillRect(x+4, y+5, 7, 4, SSD1306_WHITE);
  }
}
