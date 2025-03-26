#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C  // Common OLED I2C address
#define OLED_SDA      21     // Adjusted for new OLED
#define OLED_SCL      22

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

  // Display initial message
  display.println("Initializing...");
  display.display();
  
  // Simulating blockchain interactions
  delay(2000);  // Wait for a bit

  // Simulate displaying the current blockchain
  handleGetChain();
  delay(3000);  // Show the current blockchain for a few seconds

  // Simulate adding a new block to the blockchain
  handleAddBlock("New Block");
  delay(3000);  // Display the new blockchain with the added block
}

void loop() {
  // Nothing needed for now, everything is handled in setup
}
