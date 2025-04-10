// Define LED pins
const int redLedPin = 25;  // GPIO pin for the red LED
const int greenLedPin = 26; // GPIO pin for the green LED

void setup() {
  // Initialize the LED pins as outputs
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
}

void loop() {
  // Christmas effect: blink red and green alternately
  digitalWrite(redLedPin, HIGH);  // Turn on the red LED
  digitalWrite(greenLedPin, LOW); // Turn off the green LED
  delay(500);                     // Wait for 500ms

  digitalWrite(redLedPin, LOW);   // Turn off the red LED
  digitalWrite(greenLedPin, HIGH);// Turn on the green LED
  delay(500);                     // Wait for 500ms

  // Both LEDs on for a combined effect
  digitalWrite(redLedPin, HIGH);
  digitalWrite(greenLedPin, HIGH);
  delay(500);

  // Both LEDs off
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  delay(500);
}
