#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// —— OLED config —— //
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_ADDR     0x3C
#define OLED_RESET    -1
#define SDA_PIN       21
#define SCL_PIN       22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// —— Prototypes —— //
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
  display.display();
}

void loop() {
  display.clearDisplay();

  // —— Weather icons —— //
  drawWeatherIcon("01", 10, 12);  // Sun
  drawWeatherIcon("03", 30, 12);  // Cloud
  drawWeatherIcon("10", 50, 12);  // Rain
  drawWeatherIcon("13", 70, 12);  // Snow

  // —— Moon phases in two rows —— //
  // Phases 0–3 on top row:
  for (int p = 0; p < 4; p++) {
    drawMoonIcon(p, 10 + p * 20, 0);
  }
  // Phases 4–7 on bottom row:
  for (int p = 4; p < 8; p++) {
    drawMoonIcon(p, 10 + (p - 4) * 20, 16);
  }

  display.display();
  delay(3000);
}

// —— Improved 8×8 weather icons —— //
void drawWeatherIcon(const String& ic, int x, int y) {
  if (ic.startsWith("01")) {
    // SUN: small core + 8 rays
    display.fillCircle(x+4, y+4, 2, SSD1306_WHITE);
    const int rx[8] = {4,7,8,7,4,1,0,1};
    const int ry[8] = {0,1,4,7,8,7,4,1};
    for (int i = 0; i < 8; i++) {
      display.drawLine(x+4, y+4, x+rx[i], y+ry[i], SSD1306_WHITE);
    }
  }
  else if (ic.startsWith("02") || ic.startsWith("03") || ic.startsWith("04")) {
    // CLOUD
    display.fillCircle(x+3, y+5, 3, SSD1306_WHITE);
    display.fillCircle(x+7, y+5, 3, SSD1306_WHITE);
    display.fillRect  (x+3, y+5, 7, 4, SSD1306_WHITE);
  }
  else if (ic.startsWith("09") || ic.startsWith("10")) {
    // RAIN
    display.fillCircle(x+3, y+4, 3, SSD1306_WHITE);
    display.fillCircle(x+7, y+4, 3, SSD1306_WHITE);
    display.fillRect  (x+3, y+4, 7, 4, SSD1306_WHITE);
    display.drawLine  (x+4, y+9, x+4, y+11, SSD1306_WHITE);
    display.drawLine  (x+6, y+9, x+6, y+11, SSD1306_WHITE);
    display.drawLine  (x+8, y+9, x+8, y+11, SSD1306_WHITE);
  }
  else if (ic.startsWith("13")) {
    // SNOW
    display.fillCircle(x+3, y+4, 3, SSD1306_WHITE);
    display.fillCircle(x+7, y+4, 3, SSD1306_WHITE);
    display.fillRect  (x+3, y+4, 7, 4, SSD1306_WHITE);
    for (int dx = 4; dx <= 8; dx += 2) {
      display.drawPixel(x+dx, y+9,  SSD1306_WHITE);
      display.drawPixel(x+dx, y+11, SSD1306_WHITE);
      display.drawPixel(x+dx-1, y+10, SSD1306_WHITE);
      display.drawPixel(x+dx+1, y+10, SSD1306_WHITE);
    }
  }
  else {
    // DEFAULT: single‑puff cloud
    display.fillCircle(x+4, y+5, 3, SSD1306_WHITE);
    display.fillCircle(x+8, y+5, 3, SSD1306_WHITE);
    display.fillRect  (x+4, y+5, 7, 4, SSD1306_WHITE);
  }
}

// —— Detailed 8×8 moon phases (0=new …7=waning crescent) —— //
void drawMoonIcon(int phase, int x, int y) {
  // Outline
  display.drawCircle(x+4, y+4, 3, SSD1306_WHITE);

  switch (phase) {
    case 0: // New Moon
      display.fillCircle(x+4, y+4, 3, SSD1306_WHITE);
      break;
    case 1: // Waxing Crescent
      display.fillCircle(x+5, y+4, 2, SSD1306_WHITE);
      break;
    case 2: // First Quarter
      display.fillRect(x+4, y+1, 3, 6, SSD1306_WHITE);
      break;
    case 3: // Waxing Gibbous
      display.fillRect(x+3, y+1, 5, 6, SSD1306_WHITE);
      break;
    case 4: // Full Moon
      display.fillCircle(x+4, y+4, 3, SSD1306_WHITE);
      break;
    case 5: // Waning Gibbous
      display.fillRect(x+1, y+1, 5, 6, SSD1306_WHITE);
      break;
    case 6: // Last Quarter
      display.fillRect(x+1, y+1, 3, 6, SSD1306_WHITE);
      break;
    case 7: // Waning Crescent
      display.fillCircle(x+3, y+4, 2, SSD1306_WHITE);
      break;
  }
}
