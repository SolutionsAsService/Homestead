#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Joystick pins
#define JOY_X A0
#define JOY_Y A1
#define JOY_BUTTON 2

// Joystick thresholds
#define THRESHOLD_HIGH 800
#define THRESHOLD_LOW 200

void setup() {
  pinMode(JOY_BUTTON, INPUT_PULLUP);

  Serial.begin(9600);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Joystick Test");
  display.display();
  delay(1000);
}

void loop() {
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);
  int buttonState = digitalRead(JOY_BUTTON);

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

  switch(direction) {
    case "UP":
      display.fillTriangle(centerX, centerY - arrowSize, 
                           centerX - arrowSize, centerY + arrowSize, 
                           centerX + arrowSize, centerY + arrowSize, SSD1306_WHITE);
      break;
    case "DOWN":
      display.fillTriangle(centerX, centerY + arrowSize, 
                           centerX - arrowSize, centerY - arrowSize, 
                           centerX + arrowSize, centerY - arrowSize, SSD1306_WHITE);
      break;
    case "LEFT":
      display.fillTriangle(centerX - arrowSize, centerY, 
                           centerX + arrowSize, centerY - arrowSize, 
                           centerX + arrowSize, centerY + arrowSize, SSD1306_WHITE);
      break;
    case "RIGHT":
      display.fillTriangle(centerX + arrowSize, centerY, 
                           centerX - arrowSize, centerY - arrowSize, 
                           centerX - arrowSize, centerY + arrowSize, SSD1306_WHITE);
      break;
    default:
      display.fillCircle(centerX, centerY, 5, SSD1306_WHITE); // center
      break;
  }

  // Show raw values and button state at bottom
  display.setTextSize(1);
  display.setCursor(0, SCREEN_HEIGHT - 15);
  display.print("X: "); display.print(xVal);
  display.print(" Y: "); display.print(yVal);
  display.print(" Btn: "); display.println(buttonState == LOW ? "Pressed" : "Released");

  display.display();

  delay(100);
}
