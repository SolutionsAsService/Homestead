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

  // Draw a sun icon (icon code "01") at x=10,y=12
  drawWeatherIcon("01", 10, 12);

  // Draw a cloud icon ("03") next to it
  drawWeatherIcon("03", 30, 12);

  // Draw rain icon ("10") next to that
  drawWeatherIcon("10", 50, 12);

  // Draw snow icon ("13") next
  drawWeatherIcon("13", 70, 12);

  // Draw new moon (phase=0) at x=90,y=12
  drawMoonIcon(0, 90, 12);

  // Draw first quarter (phase=2) next
  drawMoonIcon(2, 110, 12);

  display.display();
  delay(2000);
}

// 8×8 weather icon (no trig) — you can tweak these shapes
void drawWeatherIcon(const String& ic, int x, int y) {
  if (ic.startsWith("01")) {
    // SUN: center + 8 rays
    display.fillCircle(x+4, y+4, 3, SSD1306_WHITE);
    const int rx[8] = {4,6,8,6,4,2,0,2};
    const int ry[8] = {0,2,4,6,8,6,4,2};
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

// 8×8 moon icon (phase 0…7) — simple waxing/waning shadow
void drawMoonIcon(int phase, int x, int y) {
  display.drawCircle(x+4, y+4, 3, SSD1306_WHITE);
  if (phase < 4) {
    // waxing → shadow on left
    display.fillRect(x+1, y+1, 3, 6, SSD1306_BLACK);
  } else {
    // waning → shadow on right
    display.fillRect(x+4, y+1, 3, 6, SSD1306_BLACK);
  }
}
