// Test Sketch that displays all weather icons

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// —— OLED config —— //
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_ADDR     0x3C
#define OLED_RESET    -1
#define SDA_PIN       21
#define SCL_PIN       22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// List of weather icon codes to preview
const String codes[] = {
  "01d", "02d", "03d", "04d", "09d",   // row 1
  "10d", "11d", "13d", "50d"           // row 2
};

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed");
    while (1) delay(1000);
  }
  display.clearDisplay();
}

void loop() {
  display.clearDisplay();

  // Row 1
  for (int i = 0; i < 5; i++) {
    int x = 4 + i * 24;
    drawWeatherIcon(codes[i], x, 4);
  }

  // Row 2
  for (int i = 5; i < 9; i++) {
    int j = i - 5;
    int x = 4 + j * 32;
    drawWeatherIcon(codes[i], x, 20);
  }

  display.display();
  delay(3000);
}

void drawWeatherIcon(const String& ic, int x, int y) {
  int cx = x + 4, cy = y + 4;

if (ic.startsWith("01")) {
  // SUN: slightly larger core + 8 proportionally spaced rays
  display.fillCircle(x + 4, y + 4, 3, SSD1306_WHITE);
  const int rx[8] = {4, 8, 9, 8, 4, 0, -1, 0};
  const int ry[8] = {-1, 0, 4, 8, 9, 8, 4, 0};
  for (int i = 0; i < 8; i++) {
    display.drawLine(x + 4, y + 4, x + rx[i], y + ry[i], SSD1306_WHITE);
  }
}


  else if (ic.startsWith("02")) {
    // FEW CLOUDS: sun (core radius 3) + gentle rays behind cloud
    display.fillCircle(cx - 2, cy - 2, 3, SSD1306_WHITE);
    for (int i = 0; i < 8; i++) {
      float a = i * PI / 4;
      int sx0 = cx - 2 + cos(a) * 3;
      int sy0 = cy - 2 + sin(a) * 3;
      int sx1 = cx - 2 + cos(a) * 5;
      int sy1 = cy - 2 + sin(a) * 5;
      display.drawLine(sx0, sy0, sx1, sy1, SSD1306_WHITE);
    }
    display.fillCircle(x + 5, y + 5, 3, SSD1306_WHITE);
    display.fillCircle(x + 8, y + 6, 2, SSD1306_WHITE);
    display.fillRect  (x + 5, y + 6, 6, 3, SSD1306_WHITE);
  }

  else if (ic.startsWith("03")) {
    // SCATTERED CLOUDS: three distinct puffs
    display.fillCircle(x + 2, y + 5, 3, SSD1306_WHITE);
    display.fillCircle(x + 6, y + 4, 2, SSD1306_WHITE);
    display.fillCircle(x + 8, y + 6, 2, SSD1306_WHITE);
    display.fillRect  (x + 2, y + 5, 7, 4, SSD1306_WHITE);
  }

  else if (ic.startsWith("04")) {
    // BROKEN CLOUDS: denser triple‑puff
    display.fillCircle(x + 1, y + 5, 3, SSD1306_WHITE);
    display.fillCircle(x + 5, y + 3, 3, SSD1306_WHITE);
    display.fillCircle(x + 8, y + 5, 3, SSD1306_WHITE);
    display.fillRect  (x + 1, y + 5, 9, 4, SSD1306_WHITE);
  }

  else if (ic.startsWith("09")) {
    // SHOWER RAIN: cloud + 4 drops
    display.fillCircle(x + 3, y + 4, 3, SSD1306_WHITE);
    display.fillCircle(x + 7, y + 4, 3, SSD1306_WHITE);
    display.fillRect  (x + 3, y + 4, 7, 4, SSD1306_WHITE);
    for (int i = 0; i < 4; i++) {
      display.drawLine(x + 2 + i * 2, y + 9, x + 2 + i * 2, y + 11, SSD1306_WHITE);
    }
  }

  else if (ic.startsWith("10")) {
    // RAIN: cloud + 2 drops
    display.fillCircle(x + 3, y + 4, 3, SSD1306_WHITE);
    display.fillCircle(x + 7, y + 4, 3, SSD1306_WHITE);
    display.fillRect  (x + 3, y + 4, 7, 4, SSD1306_WHITE);
    display.drawLine(x + 4, y + 9, x + 4, y + 11, SSD1306_WHITE);
    display.drawLine(x + 6, y + 9, x + 6, y + 11, SSD1306_WHITE);
  }

  else if (ic.startsWith("11")) {
  // THUNDERSTORM: cloud + more jagged lightning bolt
  display.fillCircle(x + 3, y + 4, 3, SSD1306_WHITE);
  display.fillCircle(x + 7, y + 4, 3, SSD1306_WHITE);
  display.fillRect  (x + 3, y + 4, 7, 4, SSD1306_WHITE);
  // Jagged Z-bolt
  display.drawLine(x + 5, y + 4, x + 3, y + 7, SSD1306_WHITE);
  display.drawLine(x + 3, y + 7, x + 6, y + 9, SSD1306_WHITE);
  display.drawLine(x + 6, y + 9, x + 4, y + 12, SSD1306_WHITE);
}

  else if (ic.startsWith("13")) {
    // SNOW: cloud + crisp flakes
    display.fillCircle(x + 3, y + 4, 3, SSD1306_WHITE);
    display.fillCircle(x + 7, y + 4, 3, SSD1306_WHITE);
    display.fillRect  (x + 3, y + 4, 7, 4, SSD1306_WHITE);
    for (int i = 0; i < 3; i++) {
      int px = x + 4 + i * 2, py = y + 9;
      display.drawLine(px, py, px, py + 2, SSD1306_WHITE);
      display.drawLine(px - 1, py + 1, px + 1, py + 1, SSD1306_WHITE);
    }
  }

  else if (ic.startsWith("50")) {
    // MIST
    for (int i = 0; i < 3; i++) {
      display.drawFastHLine(cx - 3, cy - 1 + i * 3, 7, SSD1306_WHITE);
    }
  }

  else {
    // DEFAULT CLOUD
    display.fillCircle(x + 4, y + 5, 3, SSD1306_WHITE);
    display.fillCircle(x + 8, y + 5, 3, SSD1306_WHITE);
    display.fillRect  (x + 4, y + 5, 7, 4, SSD1306_WHITE);
  }
}


