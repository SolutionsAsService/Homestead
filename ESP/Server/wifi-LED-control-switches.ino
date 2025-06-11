#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <pgmspace.h>

// Wi-Fi credentials
const char* ssid     = "emeraldcity";
const char* password = "spacemonkeys";

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define OLED_ADDR     0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// LED pin assignments
const int redLedPin   = 25;
const int greenLedPin = 26;
const int blueLedPin  = 2;  // Onboard LED

// LED states
bool redState   = false;
bool greenState = false;
bool blueState  = false;

// Web server
WebServer server(80);

// HTML content
static const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 LED Control</title>
  <style>
    body { font-family: Arial; text-align: center; padding-top: 30px; }
    .switch { position: relative; display: inline-block; width: 60px; height: 34px; }
    .switch input { display: none; }
    .slider {
      position: absolute; cursor: pointer;
      top: 0; left: 0; right: 0; bottom: 0;
      background-color: #ccc;
      transition: .4s; border-radius: 34px;
    }
    .slider:before {
      position: absolute; content: "";
      height: 26px; width: 26px;
      left: 4px; bottom: 4px;
      background-color: white;
      transition: .4s; border-radius: 50%;
    }
    input:checked + .slider {
      background-color: #4CAF50;
    }
    input:checked + .slider:before {
      transform: translateX(26px);
    }
  </style>
</head><body>
  <h1>ESP32 LED Control</h1>
  <p>Red LED: <label class="switch">
    <input type="checkbox" id="redSwitch" %RED_CHECKED% onchange="toggleLED('red', this.checked)">
    <span class="slider"></span>
  </label></p>
  <p>Green LED: <label class="switch">
    <input type="checkbox" id="greenSwitch" %GREEN_CHECKED% onchange="toggleLED('green', this.checked)">
    <span class="slider"></span>
  </label></p>
  <p>Blue LED: <label class="switch">
    <input type="checkbox" id="blueSwitch" %BLUE_CHECKED% onchange="toggleLED('blue', this.checked)">
    <span class="slider"></span>
  </label></p>
  <script>
    function toggleLED(color, state) {
      fetch(`/${color}/${state ? 'on' : 'off'}`);
    }
  </script>
</body></html>
)rawliteral";

// OLED IP + status display
void updateOLED() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("Red:   %s\n", redState   ? "ON" : "OFF");
  display.printf("Green: %s\n", greenState ? "ON" : "OFF");
  display.printf("Blue:  %s\n", blueState  ? "ON" : "OFF");
  display.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  display.display();
}

// Serve main HTML
void handleRoot() {
  String page = FPSTR(htmlPage);
  page.replace("%RED_CHECKED%",   redState   ? "checked" : "");
  page.replace("%GREEN_CHECKED%", greenState ? "checked" : "");
  page.replace("%BLUE_CHECKED%",  blueState  ? "checked" : "");
  server.send(200, "text/html", page);
}

// Toggle any LED state
void setLEDState(int pin, bool& state, bool newState) {
  state = newState;
  digitalWrite(pin, newState ? HIGH : LOW);
  updateOLED();
  server.send(200, "text/plain", newState ? "ON" : "OFF");
}

// Individual handlers
void handleRedOn()    { setLEDState(redLedPin,   redState,   true); }
void handleRedOff()   { setLEDState(redLedPin,   redState,   false); }
void handleGreenOn()  { setLEDState(greenLedPin, greenState, true); }
void handleGreenOff() { setLEDState(greenLedPin, greenState, false); }
void handleBlueOn()   { setLEDState(blueLedPin,  blueState,  true); }
void handleBlueOff()  { setLEDState(blueLedPin,  blueState,  false); }

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED init failed");
    while (true) delay(1000);
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  pinMode(redLedPin,   OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin,  OUTPUT);
  digitalWrite(redLedPin,   LOW);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(blueLedPin,  LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected");
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();

  // Route setup
  server.on("/",         handleRoot);
  server.on("/red/on",   handleRedOn);
  server.on("/red/off",  handleRedOff);
  server.on("/green/on", handleGreenOn);
  server.on("/green/off",handleGreenOff);
  server.on("/blue/on",  handleBlueOn);
  server.on("/blue/off", handleBlueOff);
  server.begin();

  Serial.println("Web server started");
  Serial.println(WiFi.localIP());

  updateOLED();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) WiFi.reconnect();
  server.handleClient();
}
