/*
  Clock Display on AE BTG 091OLEDW 128x32 OLED
  (I2C interface using default pins: SDA=21, SCL=22)
  US Mountain Time (MST7MDT)
  
  This code connects to Wi-Fi, syncs time via NTP,
  then continuously displays the current time and date
  on both the Serial Monitor and the OLED display.
  
  Required libraries:
    - Adafruit SSD1306
    - Adafruit GFX Library
*/

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display configuration
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels
#define OLED_SDA_PIN 21   // I2C SDA pin (ESP32 default)
#define OLED_SCL_PIN 22   // I2C SCL pin (ESP32 default)
#define OLED_ADDR    0x3C // I2C address for the OLED
#define OLED_RESET   -1   // Reset pin (not used, so set to -1)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Wi-Fi credentials
const char* ssid     = "emeraldcity";
const char* password = "spacemonkeys";

// US Mountain Time zone (POSIX format for MST/MDT)
const char* timezone = "MST7MDT";

// NTP sync interval (in milliseconds) - 30 minutes
const unsigned long ntpSyncInterval = 30UL * 60UL * 1000UL;
unsigned long lastNTPUpdate = 0;

void syncTime() {
  Serial.print("Synchronizing time with NTP server...");
  // Configure NTP servers for time synchronization
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  
  time_t now = time(nullptr);
  // Wait for a valid time (at least 24 hours from epoch)
  while (now < 24 * 3600) {
    delay(100);
    now = time(nullptr);
  }
  Serial.println(" Time synchronized!");
  
  // Set timezone and update runtime environment
  setenv("TZ", timezone, 1);
  tzset();
  
  lastNTPUpdate = millis();
}

void setup() {
  Serial.begin(115200);
  
  // Initialize I2C with SDA on GPIO21 and SCL on GPIO22
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  
  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Halt if OLED initialization fails
  }
  display.clearDisplay();
  display.display();
  
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected.");
  
  // Synchronize time via NTP
  syncTime();
}

void loop() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  
  // Print current time and date to the Serial Monitor
  Serial.printf("Current time: %02d:%02d:%02d, Date: %04d-%02d-%02d\n",
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
  
  // Clear the OLED display buffer
  display.clearDisplay();
  
  // Draw a border for a neat look
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  
  // Display the time with a larger text size
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 5);
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  display.println(timeStr);
  
  // Display the date in smaller text below the time
  display.setTextSize(1);
  display.setCursor(34, 22);
  char dateStr[12];
  sprintf(dateStr, "%04d-%02d-%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
  display.println(dateStr);
  
  // Write the display buffer to the OLED
  display.display();
  
  // Re-sync with NTP every 30 minutes to keep the time accurate
  if (millis() - lastNTPUpdate > ntpSyncInterval) {
    syncTime();
  }
  
  delay(1000); // Update the display every second
}
