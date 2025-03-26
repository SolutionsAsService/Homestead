#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522.h>

// OLED display settings (128x32)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C  // Common OLED I2C address
#define OLED_SDA      21
#define OLED_SCL      22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RFID RC522 settings
#define RFID_SS_PIN 5   // SDA (CS)
#define RFID_RST_PIN 2  // RST
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);

// Keyes RGB LED settings (GPIO 12, 13, 14)
#define LED_RED_PIN   12
#define LED_GREEN_PIN 13
#define LED_BLUE_PIN  14

// Blockchain variables
String blockchain = "[\"Genesis Block\"]";
int blockCount = 1;  // Genesis counts as block 1

// Variables for scrolling text on OLED
String scrollLines[50]; // Array to hold wrapped lines (max 50 lines)
int totalScrollLines = 0;  // Number of lines stored
int scrollOffset = 0;      // Current scroll offset
unsigned long lastScrollTime = 0;
const unsigned long scrollInterval = 2000; // scroll update every 2 seconds
const int visibleLines = 4;  // With 32px height and text size 1, we have 4 lines

// Wraps a given message into lines of up to 'maxWidth' characters and stores them along with title lines.
void updateScrollLines(String title, String message) {
  int index = 0;
  scrollLines[index++] = title;
  scrollLines[index++] = "------------------";
  
  int maxWidth = 21; // Approx. characters per line
  int start = 0;
  while (start < message.length() && index < 50) {
    scrollLines[index++] = message.substring(start, (int)min((unsigned int)(start + maxWidth), message.length()));
    start += maxWidth;
  }
  totalScrollLines = index;
  scrollOffset = 0;
}

// Displays a "page" (visibleLines lines) from the scrollLines array, starting at scrollOffset.
void displayScrollPage() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  for (int i = 0; i < visibleLines; i++) {
    int lineIndex = scrollOffset + i;
    if (lineIndex < totalScrollLines) {
      display.println(scrollLines[lineIndex]);
    }
  }
  display.display();
}

// Adds a new block to the blockchain and updates the scrollLines for display.
void addBlock(String tagID) {
  blockCount++;
  String newBlock = "Block " + String(blockCount) + ": " + tagID;
  // Append new block into our blockchain string (JSON-like format)
  blockchain = blockchain.substring(0, blockchain.length() - 1) + ",\"" + newBlock + "\"]";
  updateScrollLines("Blockchain Updated", blockchain);
  displayScrollPage();
}

// Checks for RFID tag scans and processes them.
void checkRFID() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String tagID = "";
    // Build tagID string from UID bytes
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if (mfrc522.uid.uidByte[i] < 0x10) tagID += "0";
      tagID += String(mfrc522.uid.uidByte[i], HEX);
    }
    tagID.toUpperCase();
    
    // Immediately display the scanned tag
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(4,4);
    display.println("RFID Scanned:");
    display.println("Tag: " + tagID);
    display.display();
    delay(1000);
    
    // Add the scanned tag to the blockchain
    addBlock(tagID);
    
    // Light up the LED green to indicate success (green on LED_GREEN_PIN)
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_BLUE_PIN, LOW);
    delay(2000);  // Hold the green light for 2 seconds
    digitalWrite(LED_GREEN_PIN, LOW);
    
    // Halt RFID processing for this card
    mfrc522.PICC_HaltA();
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize LED pins
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_BLUE_PIN, LOW);

  // Initialize I2C for OLED and OLED itself
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED allocation failed");
    while (true);
  }
  display.clearDisplay();
  display.display();

  // Initialize RFID module via SPI
  SPI.begin();
  mfrc522.PCD_Init();

  // Set up initial blockchain display
  updateScrollLines("Blockchain", blockchain);
  displayScrollPage();
  lastScrollTime = millis();
}

void loop() {
  // Check for RFID scans
  checkRFID();

  // Update scroll offset if enough time has passed
  if (millis() - lastScrollTime > scrollInterval) {
    // Only scroll if total lines exceed the visible lines
    if (totalScrollLines > visibleLines) {
      scrollOffset++;
      if (scrollOffset > totalScrollLines - visibleLines) {
        scrollOffset = 0;
      }
      displayScrollPage();
    }
    lastScrollTime = millis();
  }
}
