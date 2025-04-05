/*********
  Master code that continuously scans the I2C bus and displays a live node count
  and connected device addresses on an OLED.
  
  Required libraries:
    - Wire.h
    - Adafruit_GFX.h
    - Adafruit_SSD1306.h
*********/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_ADDR     0x3C
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // Initialize I2C bus
  Wire.begin();
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true); // Halt execution if display initialization fails
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  int nodeCount = 0;
  String nodesFound = "";
  
  // Scan common I2C address range (0x08 to 0x77)
  for (uint8_t address = 8; address < 120; address++) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      nodeCount++;
      nodesFound += "0x" + String(address, HEX) + " ";
      Serial.printf("I2C device found at address 0x%X\n", address);
    }
  }
  
  // Update OLED display with the current node count and addresses
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("I2C Live Scan:");
  display.print("Count: ");
  display.println(nodeCount);
  display.println("Addr:");
  display.println(nodesFound);
  display.display();
  
  // Debug output to serial monitor
  Serial.printf("Total nodes detected: %d\n", nodeCount);
  Serial.println(nodesFound);
  
  // Update scan every second for a real-time display
  delay(1000);
}
