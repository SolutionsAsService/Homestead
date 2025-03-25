#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522.h>

// OLED display settings for a 128x32 mini OLED
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32 
#define OLED_RESET    -1   // Not used on many mini OLEDs
#define SCREEN_ADDRESS 0x3C // I2C address for the OLED

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RFID RC522 settings
#define RST_PIN 2  // RFID module Reset pin connected to GPIO2
#define SS_PIN  5  // RFID module SDA (CS) connected to GPIO5
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

void setup() {
  Serial.begin(115200);
  while (!Serial);  // Optional: wait for serial monitor
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("RFID System Ready");
  display.display();
  
  // Initialize SPI for RFID module and then the RFID reader
  SPI.begin();  
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized");
}

void loop() {
  // Look for a new RFID card
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  // When a card is detected, display "Access Granted"
  displayAccessGranted();
  
  // Optionally, print UID to Serial Monitor
  Serial.print("Card UID:");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(" ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();
  
  // Halt the current card and stop encryption
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
  delay(3000); // Keep the message for 3 seconds
  
  // Return to idle message
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("RFID System Ready");
  display.display();
}

void displayAccessGranted() {
  display.clearDisplay();
  display.setTextSize(2);  // Increase text size for emphasis
  display.setCursor(0, 0);
  display.print("Access");
  display.setCursor(0, 20);
  display.print("Granted");
  display.display();
}
