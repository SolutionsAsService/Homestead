#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522.h>

// OLED display settings for a 128x32 mini OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RFID RC522 settings
#define RST_PIN 2
#define SS_PIN  5
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Fantasy name components
const char* firstPrefixes[] = {
  "Aer", "Drak", "Eld", "Gor", "Thal", "Xan", "Zar", "Mor", "Syl", "Val", "Orin", "Krev", "Fael"
};
const char* firstSuffixes[] = {
  "ion", "dor", "an", "en", "ar", "us", "iel", "or", "ith", "ius"
};

const char* lastPrefixes[] = {
  "Storm", "Shadow", "Iron", "Moon", "Frost", "Night", "Dawn", "Stone", "Wolf", "Dragon", "Silver", "Golden", "Blood"
};
const char* lastSuffixes[] = {
  "blade", "singer", "runner", "breaker", "watcher", "ward", "keeper", "forge", "heart", "bane", "song", "helm", "crest"
};

String generateFantasyName() {
  // Generate first name
  int fp = random(0, sizeof(firstPrefixes) / sizeof(firstPrefixes[0]));
  int fs = random(0, sizeof(firstSuffixes) / sizeof(firstSuffixes[0]));
  String firstName = String(firstPrefixes[fp]) + String(firstSuffixes[fs]);
  
  // Generate last name
  int lp = random(0, sizeof(lastPrefixes) / sizeof(lastPrefixes[0]));
  int ls = random(0, sizeof(lastSuffixes) / sizeof(lastSuffixes[0]));
  String lastName = String(lastPrefixes[lp]) + String(lastSuffixes[ls]);
  
  return firstName + " " + lastName;
}

void setup() {
  Serial.begin(115200);
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(2, 2);
  display.print("RFID Fantasy Name Gen Ready");
  display.display();
  
  // Initialize SPI for RFID module and the RFID reader
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized");
  
  // Seed random generator using analog read (unconnected analog pin)
  randomSeed(analogRead(A0));
}

void loop() {
  // Look for a new RFID card
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  // Generate a random fantasy name and display it
  String fullName = generateFantasyName();
  displayFantasyName(fullName);
  
  // Optionally, print UID to Serial Monitor
  Serial.print("Card UID:");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(" ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();
  
  // Halt the card and stop encryption
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
  // Wait 5 seconds for user to read the name
  delay(5000);
  
  // Reset display back to idle message
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("RFID Fantasy Name Gen Ready");
  display.display();
}

void displayFantasyName(String name) {
  // Clear the display and prepare for two-line output
  display.clearDisplay();
  display.setTextSize(1);
  
  // Find the space in the name (first name and last name separator)
  int spaceIndex = name.indexOf(' ');
  String firstName = name.substring(0, spaceIndex);
  String lastName = name.substring(spaceIndex + 1);
  
  // Display first name on the first line
  display.setCursor(4, 2);
  display.print(firstName);
  // Display last name on the second line (approx. 16 pixels down)
  display.setCursor(4, 18);
  display.print(lastName);
  display.display();
}
