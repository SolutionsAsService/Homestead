#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define display dimensions for the mini OLED (128x32)
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32 
#define OLED_RESET    -1      // Not used for many mini OLEDs
#define SCREEN_ADDRESS 0x3C   // Common I2C address for these OLEDs

// Create the OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Array of Otto von Bismarck quotes
const char* quotes[] = {
  "One day the great European War will come out of some damned foolish thing in the Balkans (1888).",
  "Laws are like sausages. It is better not to see them being made.",
  "What we learn from History is that no one learns from History",
  "God has a special providence for fools, drunks, and the United States of America.",
  "People never lie so much as after a hunt, during a war or before an election.",
  "Only a fool learns from his own mistakes. The wise man learns from the mistakes of others.",
  "The division of the United States into federations of equal force was decided long before the Civil War by the high financial powers of Europe. These bankers were afraid that the United States, if they remained in one block and as one nation, would attain economic and financial independence, which would upset their financial domination over the world. The voice of the Rothschilds prevailed... Therefore they sent their emissaries into the field to exploit the question of slavery and to open an abyss between the two sections of the Union.",
  "Politics is the art of the possible.",
  "The Americans are a very lucky people. They're bordered to the north and south by weak neighbors, and to the east and west by fish.",
  "The great questions of the day will not be settled by means of speeches and majority decisions but by iron and blood.",
  "The death of Lincoln was a disaster for Christendom. There was no man in the United States great enough to wear his boots and the bankers went anew to grab the riches. I fear that foreign bankers with their craftiness and tortuous tricks will entirely control the exuberant riches of America and use it to systematically corrupt civilization.",
  "Whoever desires peace, let him prepare for war.",
  "The great questions of the day will not be settled by speeches and majority decisions but by iron and blood.",
  "If you want to test the loyalty of a nation, give it power."
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
  display.setTextSize(1);           // Small text size for one-line display
  display.setTextWrap(false);       // Disable automatic text wrapping
  
  // Seed random number generator using an unconnected analog pin
  randomSeed(analogRead(0));
}

void loop() {
  // Randomly select one Otto von Bismarck quote and scroll it once
  int index = random(numQuotes);
  scrollText(quotes[index]);
  delay(1000); // Pause 1 second between quotes
}

// Function to scroll a text string from right to left across the OLED once
void scrollText(const char* text) {
  display.clearDisplay();
  
  // Get text bounds (this returns the width and height of the text)
  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  display.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);
  
  // Start text off-screen to the right
  int x = SCREEN_WIDTH;
  // Center text vertically on the 32-pixel-high screen
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
  
  // Clear display after scrolling
  display.clearDisplay();
  display.display();
}
