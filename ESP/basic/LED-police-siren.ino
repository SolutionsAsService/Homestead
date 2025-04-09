// ESP32 Police‑Siren‑Style Blink
// Onboard LED (usually GPIO2) flashes in two bursts of 3 quick blinks

const int LED_PIN = 2;            // Onboard LED pin
const int flashCount = 3;         // Number of quick flashes per burst
const int flashOn = 100;          // ms LED stays ON each flash
const int flashOff = 100;         // ms LED stays OFF between flashes
const int pauseBetweenBursts = 300;  // ms pause between the two bursts
const int pauseAfterBursts = 600;    // ms pause before repeating the pattern

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // First burst of quick flashes
  for (int i = 0; i < flashCount; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(flashOn);
    digitalWrite(LED_PIN, LOW);
    delay(flashOff);
  }

  // Short pause before second burst
  delay(pauseBetweenBursts);

  // Second burst of quick flashes
  for (int i = 0; i < flashCount; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(flashOn);
    digitalWrite(LED_PIN, LOW);
    delay(flashOff);
  }

  // Longer pause before the pattern repeats
  delay(pauseAfterBursts);
}
