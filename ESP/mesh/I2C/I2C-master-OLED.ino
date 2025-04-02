/*********
  Master code with OLED display showing dynamic I²C node count.
  
  This sketch initializes an SSD1306 128x32 OLED display,
  scans the I²C bus for active devices, displays the total count,
  and communicates with each detected node.
  
  Required libraries:
    - Wire.h
    - Adafruit_GFX.h
    - Adafruit_SSD1306.h
*********/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_ADDR     0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint32_t messageCount = 0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Wire.begin();

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true); // Loop forever if OLED init fails.
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("I2C Master Init");
  display.display();
  delay(1000);
}

void loop() {
  int nodeCount = 0;

  // Scan the I²C bus for connected nodes
  for (uint8_t address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      nodeCount++;
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      delay(5); // brief delay between devices
    }
  }

  Serial.print("Total I2C devices: ");
  Serial.println(nodeCount);

  // Update OLED with the current node count
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("I2C Master");
  display.print("Nodes: ");
  display.println(nodeCount);
  display.print("Msg Count:");
  display.println(messageCount);
  display.display();

  // Communicate with each detected node
  for (uint8_t address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      // Send a greeting message to this node
      Wire.beginTransmission(address);
      Wire.printf("Hello from master #%lu", messageCount++);
      error = Wire.endTransmission();
      Serial.print("Sent to address 0x");
      Serial.print(address, HEX);
      Serial.print(" error: ");
      Serial.println(error);

      // Request a response from the slave (up to 16 bytes)
      uint8_t bytesReceived = Wire.requestFrom(address, (uint8_t)16);
      Serial.print("Response from 0x");
      Serial.print(address, HEX);
      Serial.print(" (");
      Serial.print(bytesReceived);
      Serial.println(" bytes): ");
      while (Wire.available()) {
        char c = Wire.read();
        Serial.print(c);
      }
      Serial.println();
    }
  }

  // Wait 5 seconds before scanning again
  delay(5000);
}
