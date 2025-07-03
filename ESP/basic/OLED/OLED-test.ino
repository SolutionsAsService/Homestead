#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1  // Reset pin not used on many OLEDs

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);

  // Initialize the OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // 0x3C is typical I2C address
    Serial.println(F("OLED initialization failed"));
    while (true);  // Stay here forever
  }

  // Clear the display buffer
  display.clearDisplay();

  // Show startup message
  display.setTextSize(1);       // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);      // Start at top-left corner
  display.println(F("OLED Test OK"));
  display.println(F("ESP32 + SSD1306"));
  display.display();            // Actually draw the text

  delay(2000);

  // Show a test animation: scrolling box
  for (int16_t x = 0; x < SCREEN_WIDTH; x += 5) {
    display.clearDisplay();
    display.drawRect(x, 5, 20, 20, SSD1306_WHITE);
    display.display();
    delay(50);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Done!"));
  display.display();
}

void loop() {
  // Nothing here
}
