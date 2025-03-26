#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define display dimensions for the mini OLED (128x32)
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32 
#define OLED_RESET    -1    // Not used for many mini OLEDs
#define SCREEN_ADDRESS 0x3C // Common I2C address for these OLEDs

// Create the OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Array of Terry Davis–inspired single-line quotes
const char* quotes[] = {
  "Its good to be king, wait, maybe - I think maybe I'm just a bizarre person who walks back and forth, whatever.",
  "I think teeth remind us of mortality.",
  "I am the temple. I walk back and forth.",
  "TempleOS is divine; every line is blessed.",
  "Programming is sacred - walking the line."
};
const int numQuotes = sizeof(quotes) / sizeof(quotes[0]);

// Scroll parameters
const int scrollSpeed = 2;    // Pixels per iteration
const int scrollDelay = 30;   // Delay in milliseconds

void setup() {
  // Initialize I2C on ESP32: SDA = GPIO21, SCL = GPIO22
  Wire.begin(21, 22);
  Serial.begin(115200);
  
  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Halt if display initialization fails
  }
  
  // Clear display and set text properties
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);           // Small text for one-line display
  display.setTextWrap(false);       // Disable automatic text wrapping
  
  // Seed random number generator using an unconnected analog pin
  randomSeed(analogRead(0));
}

void loop() {
  // Randomly select one quote and scroll it once
  int index = random(numQuotes);
  scrollText(quotes[index]);
  delay(1000); // Pause 1 second between quotes
}

// Function to scroll a text string from right to left across the OLED once
void scrollText(const char* text) {
  display.clearDisplay();
  
  // Get text bounds without wrapping
  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  display.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);
  
  // Start the text off-screen to the right
  int x = SCREEN_WIDTH;
  // Center text vertically on a 32-pixel-high screen
  int y = (SCREEN_HEIGHT - textHeight) / 2;
  
  // Scroll text until it is completely off the left side
  while (x > -((int)textWidth)) {
    display.clearDisplay();
    display.setCursor(x, y);
    display.print(text);
    display.display();
    delay(scrollDelay);
    x -= scrollSpeed;
  }
  
  // Clear display at the end of scrolling so no ghost remains
  display.clearDisplay();
  display.display();
}
