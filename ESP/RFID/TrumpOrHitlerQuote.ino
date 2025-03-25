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

// RGB LED pins (Changed to 13, 14, 15)
#define RED_PIN   13
#define GREEN_PIN 14
#define BLUE_PIN  15

// Quotes arrays
const char* trumpQuotes[] = {
  "I will be the greatest jobs president that God ever created.",
  "I have a great relationship with the blacks. I've always had a great relationship with the blacks.",
  "I know words, I have the best words."
};

const char* hitlerQuotes[] = {
  "The great masses of the people will more easily fall victims to a big lie than to a small one.",
  "If you tell a big enough lie and tell it frequently enough, it will be believed.",
  "He alone, who owns the youth, gains the future."
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
  display.print("RFID System Ready");
  display.display();

  // Initialize SPI for RFID module
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized");

  // Initialize RGB LED pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  turnOffLEDs();
  
  // Initialize random seed
  randomSeed(analogRead(A0));
}

void loop() {
  // Look for a new RFID card
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Generate a random quote and display it
  int speaker = random(2); // 0 for Trump, 1 for Hitler
  displayRandomQuote(speaker);

  // Wait 5 seconds before showing LED result
  delay(5000);

  // Light up LED based on the speaker
  lightUpLED(speaker);

  // Wait 2 seconds, then reset display and LEDs
  delay(2000);
  resetDisplay();
  turnOffLEDs();
}

void displayRandomQuote(int speaker) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  if (speaker == 0) { // Trump quote
    int index = random(0, sizeof(trumpQuotes) / sizeof(trumpQuotes[0]));
    display.print(trumpQuotes[index]);
  } else { // Hitler quote
    int index = random(0, sizeof(hitlerQuotes) / sizeof(hitlerQuotes[0]));
    display.print(hitlerQuotes[index]);
  }
  display.display();
}

void lightUpLED(int speaker) {
  if (speaker == 0) {
    // Green for Trump
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 255);
    analogWrite(BLUE_PIN, 0);
  } else {
    // Red for Hitler
    analogWrite(RED_PIN, 255);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 0);
  }
}

void turnOffLEDs() {
  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 0);
}

void resetDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("RFID System Ready");
  display.display();
}
