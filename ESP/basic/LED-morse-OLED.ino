#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ——— CONFIG —————————————————————————————————————————————
#define LED_PIN       2       // onboard LED
#define UNIT_TIME     500     // length of one Morse “dot” in ms

// I²C pins for “normal” ESP32 usage
#define SDA_PIN       21
#define SCL_PIN       22

// OLED display size
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  32
#define OLED_RESET    -1      // not used

// Morse lookup
const char* letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const char* morseCode[] = {
  ".-",   "-...", "-.-.", "-..",  ".",    "..-.", "--.",  "....", // A–H
  "..",   ".---", "-.-",  ".-..", "--",   "-.",   "---",  ".--.", // I–P
  "--.-", ".-.",  "...",  "-",    "..-",  "...-", ".--",  "-..-", // Q–X
  "-.--", "--..",                                           // Y–Z
  "-----", ".----","..---","...--","....-",".....","-....","--...","---..","----." // 0–9
};

// Split your message into words for easy display
const char* words[] = {
  "I", "LOVE", "YOU", "JAIME", "ALWAYS", "AND", "FOREVER"
};
const int numWords = sizeof(words) / sizeof(words[0]);

// ——— OLED OBJECT ——————————————————————————————————————————
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ——— LOW‑LEVEL BLINK FUNCTIONS ———————————————————————————
void dot() {
  digitalWrite(LED_PIN, HIGH);
  delay(UNIT_TIME);
  digitalWrite(LED_PIN, LOW);
  delay(UNIT_TIME);  // intra‑element gap
}

void dash() {
  digitalWrite(LED_PIN, HIGH);
  delay(3 * UNIT_TIME);
  digitalWrite(LED_PIN, LOW);
  delay(UNIT_TIME);  // intra‑element gap
}

// Send one alphanumeric character in Morse (assumes A–Z,0–9 only)
void sendMorseChar(char c) {
  c = toupper(c);
  // find the code
  const char* code = "";
  for (int i = 0; i < (int)strlen(letters); i++) {
    if (letters[i] == c) {
      code = morseCode[i];
      break;
    }
  }
  // blink each “.” or “-”
  for (int i = 0; code[i] != '\0'; i++) {
    if (code[i] == '.')      dot();
    else if (code[i] == '-') dash();
    // (each dot/dash leaves a 1‑unit gap)
  }
  // inter‑letter gap: we've already done 1 unit after the last symbol,
  // so add 2 more to total 3 units
  delay(2 * UNIT_TIME);
}

// ——— SETUP ———————————————————————————————————————————————
void setup() {
  pinMode(LED_PIN, OUTPUT);

  // init I²C on chosen pins
  Wire.begin(SDA_PIN, SCL_PIN);

  // init display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // OLED not found: hang here
    for (;;) ;
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

// ——— MAIN LOOP ———————————————————————————————————————————
void loop() {
  // iterate through each word
  for (int w = 0; w < numWords; w++) {
    const char* word = words[w];

    // for each letter in the word
    for (int i = 0; word[i] != '\0'; i++) {
      char c = word[i];

      // 1) update OLED: small font for word, large font for letter
      display.clearDisplay();

      // top line: current word
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Word: ");
      display.print(word);

      // bottom: big letter
      display.setTextSize(2);
      display.setCursor(0, 12);
      display.print(c);

      display.display();

      // 2) blink it in Morse
      sendMorseChar(c);
    }

    // inter‑word gap: want 7 units total.
    // Since last sendMorseChar added 3 units letter‑gap,
    // add only 4 more units here.
    delay(4 * UNIT_TIME);
  }

  // after finishing all words, pause a bit before restarting
  delay(7 * UNIT_TIME);
}
