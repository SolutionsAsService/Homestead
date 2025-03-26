#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522.h>

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C  // Common OLED I2C address
#define OLED_SDA      21
#define OLED_SCL      22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RFID RC522 settings
#define RFID_SS_PIN 5   // SDA (CS)
#define RFID_RST_PIN 2  // RST
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);

// Keyes RGB LED settings (changed to GPIO 12-14)
#define LED_RED_PIN   12
#define LED_GREEN_PIN 13
#define LED_BLUE_PIN  14

// Simple blockchain representation as a JSON array string
String blockchain = "[\"Genesis Block\"]";
int blockCount = 1;  // Genesis block counts as block 1

// Function to wrap and display text on the OLED
void displayWrappedText(String title, String message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  // Print title and separator
  display.println(title);
  display.println("------------------");

  int maxWidth = 21; // Approx. max characters per line
  int start = 0;
  while (start < message.length()) {
    display.println(message.substring(start, (int)min((unsigned int)(start + maxWidth), message.length())));
    start += maxWidth;
  }
  display.display();
}

// Function to add a block to the blockchain
void addBlock(String tagID) {
  blockCount++;
  String newBlock = "Block " + String(blockCount) + ": " + tagID;
  // Remove the closing bracket, add new block, then close the array
  blockchain = blockchain.substring(0, blockchain.length() - 1) + ",\"" + newBlock + "\"]";
  displayWrappedText("Blockchain Updated", blockchain);
}

// Function to check for RFID scans and process them
void checkRFID() {
  // Look for new RFID card
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String tagID = "";
    // Construct tagID from UID bytes
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if (mfrc522.uid.uidByte[i] < 0x10) {
        tagID += "0";
      }
      tagID += String(mfrc522.uid.uidByte[i], HEX);
    }
    tagID.toUpperCase();
    
    // Display scanned tag
    displayWrappedText("RFID Scanned", "Tag: " + tagID);
    delay(1000);
    
    // Add the RFID tag as a new block to the blockchain
    addBlock(tagID);
    
    // Light up the LED green as proof of successful scan
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_BLUE_PIN, LOW);
    delay(2000); // Keep green light on for 2 seconds
    digitalWrite(LED_GREEN_PIN, LOW);
    
    // Halt further processing for this card
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

  // Initialize I2C for OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED allocation failed");
    while (true);
  }
  display.clearDisplay();
  display.display();

  // Initialize RFID module
  SPI.begin();
  mfrc522.PCD_Init();

  // Display initial blockchain state
  displayWrappedText("Blockchain", blockchain);
}

void loop() {
  // Continuously check for RFID scans
  checkRFID();
}
