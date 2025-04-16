// Test Sketch that displays all weather icons

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

// Prototypes
void drawWeatherIcon(const String& ic, int x, int y);

const String codes[] = {
  "01d","02d","03d","04d","09d",  // row 1
  "10d","11d","13d","50d"         // row 2
};

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

  // Row 1: codes[0..4]
  for (int i = 0; i < 5; i++) {
    int x = 4 + i * 24;   // 5 icons, spaced 24px apart
    drawWeatherIcon(codes[i], x, 4);
  }
  // Row 2: codes[5..8]
  for (int i = 5; i < 9; i++) {
    int x = 4 + (i-5) * 30;  // 4 icons, spaced 30px apart
    drawWeatherIcon(codes[i], x, 20);
  }

  display.display();
  delay(3000);
}

// Covers icon prefixes 01–04,09–11,13,50
void drawWeatherIcon(const String& ic, int x, int y) {
  int cx = x + 4, cy = y + 4;

  if (ic.startsWith("01")) {
    // CLEAR SKY
    display.fillCircle(cx, cy, 3, SSD1306_WHITE);
    for (int i = 0; i < 8; i++) {
      float a = i * PI/4;
      display.drawLine(
        cx + cos(a)*4, cy + sin(a)*4,
        cx + cos(a)*6, cy + sin(a)*6,
        SSD1306_WHITE
      );
    }
  }
  else if (ic.startsWith("02")) {
    // FEW CLOUDS
    display.fillCircle(cx-1, cy-1, 2, SSD1306_WHITE);
    display.fillCircle(cx+2, cy+2, 3, SSD1306_WHITE);
    display.fillCircle(cx+5, cy+2, 3, SSD1306_WHITE);
    display.fillRect  (cx+1, cy+2, 6, 4, SSD1306_WHITE);
  }
  else if (ic.startsWith("03")) {
    // SCATTERED CLOUDS
    display.fillCircle(cx+2, cy+2, 3, SSD1306_WHITE);
    display.fillCircle(cx+6, cy+2, 3, SSD1306_WHITE);
    display.fillRect  (cx+2, cy+2, 7, 4, SSD1306_WHITE);
  }
  else if (ic.startsWith("04")) {
    // BROKEN CLOUDS
    display.fillCircle(cx+1, cy+3, 3, SSD1306_WHITE);
    display.fillCircle(cx+5, cy+1, 3, SSD1306_WHITE);
    display.fillRect  (cx+1, cy+3, 7, 4, SSD1306_WHITE);
  }
  else if (ic.startsWith("09")) {
    // SHOWER RAIN
    display.fillCircle(cx+2, cy+2, 3, SSD1306_WHITE);
    display.fillCircle(cx+6, cy+2, 3, SSD1306_WHITE);
    display.fillRect  (cx+2, cy+2, 7, 4, SSD1306_WHITE);
    for (int i = 0; i < 4; i++) {
      display.drawLine(cx+2+i*2, cy+8, cx+2+i*2, cy+10, SSD1306_WHITE);
    }
  }
  else if (ic.startsWith("10")) {
    // RAIN
    display.fillCircle(cx+2, cy+2, 3, SSD1306_WHITE);
    display.fillCircle(cx+6, cy+2, 3, SSD1306_WHITE);
    display.fillRect  (cx+2, cy+2, 7, 4, SSD1306_WHITE);
    display.drawLine(cx+4, cy+8, cx+4, cy+10, SSD1306_WHITE);
    display.drawLine(cx+6, cy+8, cx+6, cy+10, SSD1306_WHITE);
  }
  else if (ic.startsWith("11")) {
    // THUNDERSTORM
    display.fillCircle(cx+2, cy+2, 3, SSD1306_WHITE);
    display.fillCircle(cx+6, cy+2, 3, SSD1306_WHITE);
    display.fillRect  (cx+2, cy+2, 7, 4, SSD1306_WHITE);
    display.drawLine(cx+5, cy+4, cx+4, cy+8, SSD1306_WHITE);
    display.drawLine(cx+4, cy+8, cx+6, cy+8, SSD1306_WHITE);
    display.drawLine(cx+6, cy+8, cx+5, cy+12,SSD1306_WHITE);
  }
  else if (ic.startsWith("13")) {
    // SNOW
    display.fillCircle(cx+2, cy+2, 3, SSD1306_WHITE);
    display.fillCircle(cx+6, cy+2, 3, SSD1306_WHITE);
    display.fillRect  (cx+2, cy+2, 7, 4, SSD1306_WHITE);
    for (int i = 0; i < 3; i++) {
      int px = cx+3 + i*2, py = cy+8;
      display.drawPixel(px,   py,   SSD1306_WHITE);
      display.drawPixel(px,   py+2, SSD1306_WHITE);
      display.drawPixel(px-1, py+1, SSD1306_WHITE);
      display.drawPixel(px+1, py+1, SSD1306_WHITE);
    }
  }
  else if (ic.startsWith("50")) {
    // MIST
    for (int i = 0; i < 3; i++) {
      display.drawFastHLine(cx-3, cy-1 + i*3, 7, SSD1306_WHITE);
    }
  }
  else {
    // DEFAULT CLOUD
    display.fillCircle(cx+2, cy+2, 3, SSD1306_WHITE);
    display.fillCircle(cx+6, cy+2, 3, SSD1306_WHITE);
    display.fillRect  (cx+2, cy+2, 7, 4, SSD1306_WHITE);
  }
}
