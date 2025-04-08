#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// —— User Config —— //
const char* ssid = "emeraldcity";
const char* password = "spacemonkeys";
const char* city = "Taos";
const char* country = "US";
const char* owmApiKey = "29791c1b9f625367055f374a4260a2fc";
const char* ntpServer = "pool.ntp.org";
const char* timezone = "MST7MDT"; // POSIX TZ for MST/MDT (Mountain Standard Time)

// —— Intervals —— //
const unsigned long WEATHER_INTERVAL = 10UL * 60UL * 1000UL; // 10 min
const unsigned long NTP_INTERVAL = 30UL * 60UL * 1000UL; // 30 min

// —— OLED config —— //
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_ADDR 0x3C
#define OLED_RESET -1
#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// —— State —— //
unsigned long lastWeather = 0;
unsigned long lastNTP = 0;

float tempC = 0.0;
float tempF = 0.0;
int humidity = 0;
float windSpeed = 0.0;
String weatherDesc = "";

// —— Prototypes —— //
void connectWiFi();
void ensureWiFi();
bool syncTimeBlocking();
void fetchWeather();
void drawDisplay();
void retryWeatherFetch();
void retryTimeSync();

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("ERROR: SSD1306 initialization failed");
    while (1)
      delay(1000);
  }
  display.clearDisplay();
  display.display();

  connectWiFi();

  if (!syncTimeBlocking()) {
    Serial.println("Error: Failed to synchronize time at startup.");
  }

  retryWeatherFetch(); // Ensure weather fetching is successful during setup
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
  delay(1000); // Update display every second
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to Wi‑Fi: \"%s\"…\n", ssid);
  
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) { // 10-second timeout
    Serial.print('.');
    delay(500);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nConnected to Wi‑Fi, IP=%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWi‑Fi connection failed. Retrying in loop.");
  }
}

void ensureWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi‑Fi lost — reconnecting");
    WiFi.disconnect();
    connectWiFi();
  }
}

bool syncTimeBlocking() {
  Serial.println("Synchronizing NTP time (blocking)…");
  setenv("TZ", timezone, 1); // Set POSIX timezone
  tzset();
  configTime(0, 0, ntpServer);

  struct tm tm;
  int retryCount = 0;

  while (!getLocalTime(&tm, 10000) && retryCount < 3) { // Retry up to 3 times
    Serial.printf("Retrying NTP synchronization (%d/3)…\n", retryCount + 1);
    delay(2000);
    retryCount++;
  }

  if (retryCount == 3) {
    Serial.println("NTP synchronization failed after 3 attempts.");
    return false;
  }

  Serial.printf("Time synchronized: %04d-%02d-%02d %02d:%02d:%02d\n", 
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, 
                tm.tm_hour, tm.tm_min, tm.tm_sec);
  lastNTP = millis();
  return true;
}

void retryTimeSync() {
  if (!syncTimeBlocking()) {
    Serial.println("Time synchronization failed during periodic update.");
  }
}

void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No Wi‑Fi connection — skipping weather update");
    return;
  }

  String url = String("http://api.openweathermap.org/data/2.5/weather?q=") + city + "," + country + "&units=metric&appid=" + owmApiKey;

  HTTPClient http;
  http.begin(url);
  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    String body = http.getString();
    DynamicJsonDocument doc(4096);
    auto err = deserializeJson(doc, body);
    if (!err) {
      tempC = doc["main"]["temp"].as<float>();
      tempF = tempC * 9.0 / 5.0 + 32.0;
      humidity = doc["main"]["humidity"].as<int>();
      windSpeed = doc["wind"]["speed"].as<float>();
      weatherDesc = doc["weather"][0]["description"].as<const char*>();
      if (weatherDesc.length() > 16)
        weatherDesc = weatherDesc.substring(0, 16);
      Serial.println("Weather data updated successfully");
    } else {
      Serial.print("JSON parsing error: ");
      Serial.println(err.c_str());
    }
  } else {
    Serial.printf("HTTP request failed with error code: %d\n", code);
  }
  http.end();
  lastWeather = millis();
}

void retryWeatherFetch() {
  int retryCount = 0;
  while (retryCount < 3) { // Retry up to 3 times
    fetchWeather();
    if (lastWeather != 0) { // Check if weather data was updated
      break;
    }
    Serial.printf("Retrying weather fetch (%d/3)…\n", retryCount + 1);
    delay(2000);
    retryCount++;
  }
  if (retryCount == 3) {
    Serial.println("Weather fetch failed after 3 attempts");
  }
}

void drawDisplay() {
  display.clearDisplay();
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE); // Border
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  struct tm tm;
  if (getLocalTime(&tm, 1000)) {
    char timeBuf[12], dateBuf[12];
    strftime(dateBuf, sizeof(dateBuf), "%m-%d-%Y", &tm); // MM-DD-YYYY format
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tm); // 24-hour format with seconds

    display.setCursor(2, 0); // Top line
    display.print(dateBuf);

    display.setCursor(2, 8); // Second line
    display.print(timeBuf);
  } else {
    display.setCursor(2, 0);
    display.print("Time N/A");
  }

  display.setCursor(2, 16); // Third line
  display.printf("T:%.1fC/%.1fF", tempC, tempF);

  display.setCursor(2, 24); // Fourth line
  display.printf("H:%d%% W:%.1fm/s", humidity, windSpeed);
  display.printf(" %s", weatherDesc.c_str());

  display.display();
}
