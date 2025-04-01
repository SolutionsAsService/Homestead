#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// OLED Display setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define OLED_ADDR     0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Wi-Fi credentials
const char* ssid = "emeraldcity";
const char* password = "spacemonkeys";

// NTP Client setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -25200, 60000); // GMT offset for MST (-7 hours)

// Moon (more of a diamond rn)  icon bitmap
const uint8_t moonIcon[] PROGMEM = {
  0x00, 0x00, 0x0C, 0x00, 0x1E, 0x00, 0x3E, 0x00, 0x3F, 0x00, 0x3F, 0x80, 0x7F, 0xC0, 0x7F, 0xC0,
  0x7F, 0xC0, 0x3F, 0x80, 0x3F, 0x00, 0x3E, 0x00, 0x1E, 0x00, 0x0C, 0x00, 0x00, 0x00
};

void setup() {
  Serial.begin(115200);

  // Initialize display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 initialization failed");
    while (true);
  }
  display.clearDisplay();
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  // Start NTP time sync
  timeClient.begin();
}

void loop() {
  timeClient.update();
  
  int hour = timeClient.getHours();
  int minute = timeClient.getMinutes();
  
  // Convert to 12-hour format
  bool isPM = hour >= 12;
  hour = (hour == 0) ? 12 : (hour > 12 ? hour - 12 : hour);

  // Get timestamp for date
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);

  int month = ptm->tm_mon + 1;
  int day = ptm->tm_mday;
  int year = ptm->tm_year + 1900;

  // Format date string (MM/DD/YYYY)
  char dateStr[11];
  sprintf(dateStr, "%02d/%02d/%04d", month, day, year);

  // Format time string (HH:MM AM/PM)
  char timeStr[12];
  sprintf(timeStr, "%02d:%02d %s", hour, minute, isPM ? "PM" : "AM");

  // Clear and draw elements
  display.clearDisplay();
  display.drawRoundRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 5, SSD1306_WHITE); // Border
  display.drawBitmap(5, 5, moonIcon, 16, 16, SSD1306_WHITE); // Moon icon
  
  // Display time
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 5);
  display.println(timeStr);
  
  // Display date
  display.setTextSize(1);
  display.setCursor(34, 24);
  display.println(dateStr);
  
  display.display();
  delay(1000);
}
