#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1  // Reset pin (or -1 if sharing ESP32 reset)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  // Initialize I2C and OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Loop forever if init fails
  }
  display.clearDisplay();
  display.display();
  delay(500);
}

void drawIcons() {
  // Adjusted icon Y-coordinates to move them down
  // Prev icon ◄
  display.fillTriangle(10, 26, 20, 22, 20, 30, SSD1306_WHITE);
  // Play icon ▶
  display.fillTriangle(30, 22, 30, 30, 40, 26, SSD1306_WHITE);
  // Pause icon ❚❚
  display.fillRect(50, 22, 4, 8, SSD1306_WHITE);
  display.fillRect(58, 22, 4, 8, SSD1306_WHITE);
  // Next icon ►
  display.fillTriangle(80, 26, 70, 22, 70, 30, SSD1306_WHITE);
}

void drawText(const String &title, const String &artist) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  // Title on top line
  display.setCursor(0, 0);
  display.println(title);
  // Artist on second line
  display.setCursor(0, 10);
  display.println(artist);
}

void loop() {
  // Simulated metadata (replace with real data retrieval)
  String currentTitle  = "Song Title Example";
  String currentArtist = "Artist Name";

  display.clearDisplay();
  drawText(currentTitle, currentArtist);
  drawIcons(); // Moved icons lower
  display.display();

  // Update every second
  delay(1000);
}
