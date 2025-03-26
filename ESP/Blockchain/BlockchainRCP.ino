#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522.h>

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C  // Common OLED I2C address
#define OLED_SDA      21     // Adjusted for new OLED
#define OLED_SCL      22

// RFID RC522 settings
#define SDA_PIN 5
#define RST_PIN 2
#define SS_PIN  5
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Keyes RGB LED settings
#define RED_PIN    25
#define GREEN_PIN  26
#define BLUE_PIN   27

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Blockchain data structure
String blockchain = "[\"Genesis Block\"]";

// Function to display text wrapped on OLED
void displayWrappedText(String title, String message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  // Display title (for example, API endpoint)
  display.println(title);
  display.println("------------------");

  int maxWidth = 21; // Max characters per line
  int start = 0;

  // Display the message in lines that wrap correctly
  while (start < message.length()) {
    display.println(message.substring(start, (int)min((unsigned int)(start + maxWidth), message.length())));
    start += maxWidth;
  }
  
  display.display();
}

// Function to simulate retrieving the blockchain
void handleGetChain() {
  String response = "{\"chain\":" + blockchain + "}";
  displayWrappedText("GET /getchain", response);
}

// Function to simulate adding a new block to the blockchain
void handleAddBlock(String newBlock) {
  blockchain = blockchain.substring(0, blockchain.length() - 1) + ",\"" + newBlock + "\"]";
  String response = "{\"status\":\"Block added\",\"chain\":" + blockchain + "}";
  displayWrappedText("POST /addblock", "Block: " + newBlock);
}

// Initialize RFID reader
void initRFID() {
  SPI.begin();  // Initialize SPI bus
  mfrc522.PCD_Init();  // Initialize the MFRC522 RFID reader
}

// Function to handle RFID scan and trigger blockchain RPC
void handleRFIDScan() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String tagID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      tagID += String(mfrc522.uid.uidByte[i], HEX);
    }
    // Display RFID scan result
    displayWrappedText("RFID Scanned", "Tag ID: " + tagID);
    
    // Add a block to blockchain for the scanned tag
    handleAddBlock(tagID);  // Add tagID as a new block in the blockchain

    // Trigger the green light on the Keyes RGB LED
    digitalWrite(RED_PIN, LOW);   // Turn off red
    digitalWrite(GREEN_PIN, HIGH); // Turn on green
    digitalWrite(BLUE_PIN, LOW);  // Turn off blue
    delay(2000);  // Keep green light on for 2 seconds
    
    // Turn off the LED after showing green light
    digitalWrite(GREEN_PIN, LOW); // Turn off green
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize I2C for OLED
  Wire.begin(OLED_SDA, OLED_SCL);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);  // Set text size to small for more text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // Initialize RFID reader
  initRFID();

  // Initialize Keyes RGB LED pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Initialize LED to be off
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);

  // Display an initial message on OLED
  display.println("Initializing...");
  display.display();
  delay(2000);

  // Simulate displaying the current blockchain
  handleGetChain();
}

void loop() {
  // Continuously check for RFID scan
  handleRFIDScan();
}
