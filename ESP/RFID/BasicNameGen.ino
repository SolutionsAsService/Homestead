#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522.h>

// OLED display settings
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
const char* namePrefixes[] = {
  "Aer", "Drak", "Eld", "Gor", "Thal", "Xan", "Zar", "Mor", "Syl", "Val", "Orin", "Krev", "Fael"
};
const char* nameMiddles[] = {
  "dor", "gath", "ion", "mir", "thas", "zar", "lun", "grim", "rith", "mon", "rik", "vell", "dorn"
};
const char* nameSuffixes[] = {
  "ian", "os", "ael", "eth", "orn", "ix", "ul", "an", "ar", "en", "is", "on", "ius"
};

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
  display.setCursor(0, 0);
  display.print("Fantasy Name Gen Ready");
  display.display();

  // Initialize SPI for RFID module
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized");

  // Random seed initialization
  randomSeed(analogRead(A0));
}

void loop() {
  // Look for a new RFID card
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Generate and display a random fantasy name
  generateFantasyName();

  // Wait 3 seconds before resetting
  delay(3000);
  resetDisplay();
}

void generateFantasyName() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  // Generate a random name using prefix, middle, and suffix
  String name = String(namePrefixes[random(sizeof(namePrefixes) / sizeof(namePrefixes[0]))]) +
                String(nameMiddles[random(sizeof(nameMiddles) / sizeof(nameMiddles[0]))]) +
                String(nameSuffixes[random(sizeof(nameSuffixes) / sizeof(nameSuffixes[0]))]);

  display.print("Name: ");
  display.print(name);
  display.display();
}

void resetDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Fantasy Name Gen Ready");
  display.display();
}
