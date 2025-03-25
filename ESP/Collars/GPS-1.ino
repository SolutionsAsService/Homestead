#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

// OLED Display settings for a 128x32 mini OLED
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32 
#define OLED_RESET    -1   // Not used for many mini OLEDs
#define SCREEN_ADDRESS 0x3C // Common I2C address

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// GPS Module (Neo-6M) settings
#define RXPin 16   // Connect GPS TX to ESP32 GPIO16 (RX)
#define TXPin 17   // Connect GPS RX to ESP32 GPIO17 (TX)
#define GPSBaud 9600

// Instantiate ESPSoftwareSerial for GPS
SoftwareSerial gpsSerial(RXPin, TXPin);
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(GPSBaud);

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);  // Halt if display initialization fails
  }
  
  display.clearDisplay();
  display.setTextSize(1); // Small text for one-line display
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Waiting for GPS...");
  display.display();
  
  // Seed random number generator if needed
  randomSeed(analogRead(0));
}

void loop() {
  // Read incoming data from the GPS module using ESPSoftwareSerial
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      displayGPSData();
    }
  }
}

void displayGPSData() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  if (gps.location.isValid()) {
    display.print("Lat: ");
    display.print(gps.location.lat(), 6);
    display.setCursor(0, 10);
    display.print("Lon: ");
    display.print(gps.location.lng(), 6);
  } else {
    display.print("No GPS Fix");
  }
  
  display.display();
}
