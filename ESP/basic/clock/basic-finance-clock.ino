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
const char* owmKey     = "YOUR WEATHER KEY";
const char* avKey      = "YOUR AV API KEY";
const char* ntpServer  = "pool.ntp.org";
const char* timezone   = "MST7MDT"; // Mountain Standard/Daylight

// —— Intervals —— //
const unsigned long WEATHER_INTERVAL = 10UL * 60UL * 1000UL;  // 10 min
const unsigned long NTP_INTERVAL     = 30UL * 60UL * 1000UL;  // 30 min
const unsigned long STOCK_INTERVAL   =      60UL * 1000UL;   // 1 min

// —— OLED config —— //
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// —— State —— //
unsigned long lastWeather = 0, lastNTP = 0, lastStock = 0;
float   tempC=0, tempF=0; 
int     humidity=0; 
float   windSpeed=0;
String  weatherDesc="";
String  scrollText="";
int16_t scrollX=0, scrollW=0;

// Stock symbols
const char* symbols[] = { "AAPL", "GOOGL", "MSFT", "AMZN" };
const uint8_t symbolCount = sizeof(symbols)/sizeof(symbols[0]);

// ── Prototypes ───────────────────────────────────
void connectWiFi();
bool syncTime();
void fetchWeather();
void fetchStocks();
void drawDisplay();

void setup() {
  Serial.begin(115200);
  Wire.begin();
  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[ERROR] OLED init failed");
    while (1);
  }
  display.clearDisplay();
  connectWiFi();
  syncTime();           // initial sync
  fetchWeather();       // immediate weather
  fetchStocks();        // immediate stocks
}

void loop() {
  unsigned long now = millis();
  if (now - lastNTP   > NTP_INTERVAL)   syncTime();
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
  while (WiFi.status() != WL_CONNECTED && millis()-start < 10000) {
    delay(500);
    Serial.print('.');
  }
  if (WiFi.status()==WL_CONNECTED) {
    Serial.printf("\nWiFi up, IP=%s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWiFi failed");
  }
}

bool syncTime() {
  Serial.println("Syncing NTP…");
  configTzTime(timezone, ntpServer);
  struct tm tm;
  int retries=0;
  while (!getLocalTime(&tm) && retries<3) {
    Serial.println("  retry NTP");
    delay(2000);
    retries++;
  }
  lastNTP = millis();
  return retries<3;
}

// ── Fetch Weather ─────────────────────────────────
void fetchWeather() {
  if (WiFi.status()!=WL_CONNECTED) return;
  String url = String("http://api.openweathermap.org/data/2.5/weather?q=")
             + city + "," + country
             + "&units=metric&appid=" + owmKey;
  HTTPClient http; http.begin(url);
  int code = http.GET();
  if (code==HTTP_CODE_OK) {
    String body = http.getString();
    DynamicJsonDocument doc(4096);
    if (!deserializeJson(doc, body)) {
      tempC     = doc["main"]["temp"].as<float>();
      tempF     = tempC * 9/5 + 32;
      humidity  = doc["main"]["humidity"].as<int>();
      windSpeed = doc["wind"]["speed"].as<float>();
      weatherDesc = String((const char*)doc["weather"][0]["description"]).substring(0,10);
      Serial.println("Weather updated");
    }
  }
  http.end();
  lastWeather = millis();
}

// ── Fetch Stocks & Prepare Scroll ────────────────
void fetchStocks() {
  if (WiFi.status()!=WL_CONNECTED) return;
  scrollText = "";
  for (uint8_t i=0; i<symbolCount; i++) {
    const char* s = symbols[i];
    String url = String("https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=")
               + s + "&apikey=" + avKey;
    HTTPClient http; http.begin(url);
    int code = http.GET();
    if (code==HTTP_CODE_OK) {
      String body = http.getString();
      DynamicJsonDocument doc(512);
      if (!deserializeJson(doc, body)) {
        const char* p = doc["Global Quote"]["05. price"] | "---";
        scrollText += String(s) + ":$" + p + "   ";
      }
    }
    http.end();
    delay(200);
  }
  // compute scroll width
  int16_t x1,y1; uint16_t w,h;
  display.setTextSize(1);
  display.getTextBounds(scrollText, 0, 0, &x1, &y1, &w, &h);
  scrollW = w;
  scrollX = SCREEN_WIDTH;
  lastStock = millis();
}

// ── Draw All Lines ───────────────────────────────
void drawDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // Line 1: Time & Date
  struct tm tm;
  char buf1[20];
  if (getLocalTime(&tm)) {
    strftime(buf1, sizeof(buf1), "%I:%M %p %m-%d-%y", &tm);
  } else strcpy(buf1, "Time N/A");
  display.setCursor(0,0);
  display.print(buf1);

  // Line 2: Temp C & F
  display.setCursor(0,8);
  display.printf("T:%.1fC/%.1fF", tempC, tempF);

  // Line 3: Weather | Humidity | Wind
  display.setCursor(0,16);
  display.printf("%s H:%d%% W:%.1fm/s",
                 weatherDesc.c_str(), humidity, windSpeed);

  // Line 4: Scrolling Finance
  display.setCursor(scrollX, 24);
  display.print(scrollText);
  scrollX--;
  if (scrollX < -scrollW) scrollX = SCREEN_WIDTH;

  display.display();
}
