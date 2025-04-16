// Sketch displays several weather and moon phase icons - testing for weather clock

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

  // —— Weather icons (one row on left) —— //
  drawWeatherIcon("01",  4, 12);  // Sun
  drawWeatherIcon("03", 24, 12);  // Cloud
  drawWeatherIcon("10", 44, 12);  // Rain
  drawWeatherIcon("13", 64, 12);  // Snow

  // —— Moon phases (two rows on right) —— //
  // Top row (phases 0–3)
  for (int p = 0; p < 4; p++) {
    drawMoonIcon(p, 84 + p*10, 0);
  }
  // Bottom row (phases 4–7)
  for (int p = 4; p < 8; p++) {
    drawMoonIcon(p, 84 + (p-4)*10, 16);
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
      display.drawPixel(x+dx,  y+9,  SSD1306_WHITE);
      display.drawPixel(x+dx,  y+11, SSD1306_WHITE);
      display.drawPixel(x+dx-1,y+10, SSD1306_WHITE);
      display.drawPixel(x+dx+1,y+10, SSD1306_WHITE);
    }
  }
  else {
    // DEFAULT: single‑puff cloud
    display.fillCircle(x+4, y+5, 3, SSD1306_WHITE);
    display.fillCircle(x+8, y+5, 3, SSD1306_WHITE);
    display.fillRect  (x+4, y+5, 7, 4, SSD1306_WHITE);
  }
}

// —— Perfect half‑circle moon phases (0=new …7=waning crescent) —— //
void drawMoonIcon(int phase, int x, int y) {
  // Draw outline circle
  display.drawCircle(x+4, y+4, 3, SSD1306_WHITE);

  switch (phase) {
    case 0:
      // New Moon: empty
      break;
    case 1:
      // Waxing Crescent: small sliver on right
      display.fillCircle(x+5, y+4, 3, SSD1306_WHITE);
      display.fillCircle(x+3, y+4, 3, SSD1306_BLACK);
      break;
    case 2:
      // First Quarter: perfect right half
      display.fillCircle(x+4, y+4, 3, SSD1306_WHITE);
      display.fillCircle(x+1, y+4, 3, SSD1306_BLACK);
      break;
    case 3:
      // Waxing Gibbous: 3/4
      display.fillCircle(x+4, y+4, 3, SSD1306_WHITE);
      display.fillCircle(x+2, y+4, 3, SSD1306_BLACK);
      break;
    case 4:
      // Full Moon: fully lit
      display.fillCircle(x+4, y+4, 3, SSD1306_WHITE);
      break;
    case 5:
      // Waning Gibbous: 3/4 left
      display.fillCircle(x+4, y+4, 3, SSD1306_WHITE);
      display.fillCircle(x+6, y+4, 3, SSD1306_BLACK);
      break;
    case 6:
      // Last Quarter: perfect left half
      display.fillCircle(x+4, y+4, 3, SSD1306_WHITE);
      display.fillCircle(x+7, y+4, 3, SSD1306_BLACK);
      break;
    case 7:
      // Waning Crescent: small sliver on left
      display.fillCircle(x+3, y+4, 3, SSD1306_WHITE);
      display.fillCircle(x+5, y+4, 3, SSD1306_BLACK);
      break;
  }
}
