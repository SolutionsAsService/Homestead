#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128            // OLED width in pixels
#define SCREEN_HEIGHT 32            // OLED height in pixels
#define OLED_ADDR 0x3C              // I2C address
#define OLED_RESET -1               // Reset pin (not used)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int BUTTON_PIN = 15;          // Use GPIO15 for the button
bool lastPressed = false;           // Track last button state

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // HIGH when released, LOW when pressed

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed");
    while (true);
  }

  // Set contrast to maximum (command 0x81, value 0xFF)
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(0xFF);

  // Initial display
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 8);
  display.println("Release");
  display.display();
}

void loop() {
  bool pressed = (digitalRead(BUTTON_PIN) == LOW);

  // Only update when the state changes
  if (pressed != lastPressed) {
    display.clearDisplay();
    display.setCursor(0, 8);
    if (pressed) {
      display.println("Pressed!");
    } else {
      display.println("Release");
    }
    display.display();
    lastPressed = pressed;
  }
  // No delay needed—updates occur only on change
}
