// ESP32 Morse‑Code Blink Sketch
// Message: "I LOVE YOU JAIME ALWAYS AND FOREVER"

const int LED_PIN    = 2;       // onboard LED on most ESP32 boards
const int UNIT_TIME  = 500;     // one unit = 500 ms

// Morse code lookup: A–Z, 0–9
const char* letters   = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const char* morseCode[] = {
  ".-",   "-...", "-.-.", "-..",  ".",    "..-.", "--.",  "....", // A–H
  "..",   ".---", "-.-",  ".-..", "--",   "-.",   "---",  ".--.", // I–P
  "--.-", ".-.",  "...",  "-",    "..-",  "...-", ".--",  "-..-", // Q–X
  "-.--", "--..",                                           // Y–Z
  "-----", ".----","..---","...--","....-",".....","-....","--...","---..","----." // 0–9
};

const char* message = "I LOVE YOU JAIME ALWAYS AND FOREVER";

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

// Send one character in Morse (letter or digit)
void sendMorseChar(char c) {
  if (c == ' ') {
    // inter‑word gap: we've already just done an intra‑element gap,
    // so add 6 more units to total 7 units
    delay(6 * UNIT_TIME);
    return;
  }
  // find index in lookup
  c = toupper(c);
  const char* code = "";
  for (int i = 0; i < (int)strlen(letters); i++) {
    if (letters[i] == c) {
      code = morseCode[i];
      break;
    }
  }
  // blink each symbol
  for (int i = 0; code[i] != '\0'; i++) {
    if (code[i] == '.')      dot();
    else if (code[i] == '-') dash();
    // after each dot/dash we already delay 1 unit
  }
  // inter‑letter gap: we've already done 1 unit after last symbol,
  // so add 2 more units to total 3 units
  delay(2 * UNIT_TIME);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // send the whole message once
  for (int i = 0; message[i] != '\0'; i++) {
    sendMorseChar(message[i]);
  }
  // pause before repeating (word‑gap already applied if last char was space;
  // otherwise add full word gap)
  delay(7 * UNIT_TIME);
}
