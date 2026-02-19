#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Joystick pins
#define JOY_X 34
#define JOY_Y 35
#define BTN_B 32
#define BTN_V 33
#define BTN_G 25

// Joystick thresholds (ESP32 ADC is 0-4095)
#define THRESHOLD_HIGH 3000
#define THRESHOLD_LOW 1000

void setup() {
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_V, INPUT_PULLUP);
  pinMode(BTN_G, INPUT_PULLUP);

  Serial.begin(115200);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("5-Button Joystick ESP32");
  display.display();
  delay(1000);
}

void loop() {
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);

  bool bPressed = digitalRead(BTN_B) == LOW;
  bool vPressed = digitalRead(BTN_V) == LOW;
  bool gPressed = digitalRead(BTN_G) == LOW;

  // Determine joystick direction
  String direction = "CENTER";
  if(xVal < THRESHOLD_LOW) direction = "LEFT";
  else if(xVal > THRESHOLD_HIGH) direction = "RIGHT";
  else if(yVal < THRESHOLD_LOW) direction = "UP";
  else if(yVal > THRESHOLD_HIGH) direction = "DOWN";

  display.clearDisplay();

  // Draw arrow based on direction
  int centerX = SCREEN_WIDTH / 2;
  int centerY = SCREEN_HEIGHT / 2;
  int arrowSize = 15;

  if(direction == "UP")
    display.fillTriangle(centerX, centerY - arrowSize, centerX - arrowSize, centerY + arrowSize, centerX + arrowSize, centerY + arrowSize, SSD1306_WHITE);
  else if(direction == "DOWN")
    display.fillTriangle(centerX, centerY + arrowSize, centerX - arrowSize, centerY - arrowSize, centerX + arrowSize, centerY - arrowSize, SSD1306_WHITE);
  else if(direction == "LEFT")
    display.fillTriangle(centerX - arrowSize, centerY, centerX + arrowSize, centerY - arrowSize, centerX + arrowSize, centerY + arrowSize, SSD1306_WHITE);
  else if(direction == "RIGHT")
    display.fillTriangle(centerX + arrowSize, centerY, centerX - arrowSize, centerY - arrowSize, centerX - arrowSize, centerY + arrowSize, SSD1306_WHITE);
  else
    display.fillCircle(centerX, centerY, 5, SSD1306_WHITE); // center

  // Show raw X/Y values at bottom
  display.setTextSize(1);
  display.setCursor(0, SCREEN_HEIGHT - 15);
  display.print("X: "); display.print(xVal);
  display.print(" Y: "); display.print(yVal);

  // Show button states
  display.setCursor(0, SCREEN_HEIGHT - 30);
  display.print("B: "); display.print(bPressed ? "P" : "R");
  display.print(" V: "); display.print(vPressed ? "P" : "R");
  display.print(" G: "); display.println(gPressed ? "P" : "R");

  display.display();
  delay(100);
}
