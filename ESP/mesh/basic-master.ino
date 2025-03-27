/******************************
 * Master Node – Mesh Connection Display
 *
 * This sketch connects to WiFi with a static IP,
 * starts a TCP server on port 5000, and then every 5 seconds 
 * attempts to connect to peer nodes. It displays the total number 
 * of nodes (master + peers that respond) on an OLED.
 *
 * OLED Wiring (I²C 4-pin):
 *   VCC -> 3.3V (or 5V if supported)
 *   GND -> GND
 *   SDA -> GPIO21
 *   SCL -> GPIO22
 ******************************/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//-------------------------
// OLED Setup (I²C on GPIO21/22)
//-------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//-------------------------
// WiFi & Network Settings
//-------------------------
const char* ssid = "emeraldcity";
const char* password = "spacemonkeys";

// Master static IP
IPAddress local_IP(192,168,1,100);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

//-------------------------
// Peer Node Settings
//-------------------------
// Define an array of peer node IP addresses
IPAddress peers[] = {
  IPAddress(192,168,1,101),
  IPAddress(192,168,1,102)
};
const int numPeers = sizeof(peers) / sizeof(peers[0]);

WiFiServer server(5000);

//-------------------------
// Timing Variables
//-------------------------
unsigned long lastDisplayUpdate = 0;
const unsigned long updateInterval = 5000;  // update every 5 seconds

//-------------------------
// Check connectivity to peer nodes
//-------------------------
int checkConnectedPeers() {
  int count = 0;
  WiFiClient client;
  for (int i = 0; i < numPeers; i++) {
    if (client.connect(peers[i], 5000)) {  // try connecting on port 5000
      count++;
      client.stop();
      delay(20);  // brief pause between connections
    }
  }
  return count;
}

//-------------------------
// Update the OLED display
//-------------------------
void updateDisplay() {
  int peerCount = checkConnectedPeers();
  int totalNodes = 1 + peerCount;  // count self as 1
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 2);
  display.print("Nodes: ");
  display.println(totalNodes);
  display.setTextSize(1);
  display.println("RFID ready");
  display.display();
}

//-------------------------
// Setup
//-------------------------
void setup() {
  Serial.begin(115200);
  delay(100);
  
  // Initialize I²C for OLED on GPIO21 (SDA) and GPIO22 (SCL)
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed");
    while (true);
  }
  display.clearDisplay();
  display.display();
  
  // Configure WiFi with static IP and connect
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP().toString());
  
  // Start TCP server
  server.begin();
  Serial.println("TCP server started on port 5000");
  
  // Initial display update
  updateDisplay();
  lastDisplayUpdate = millis();
}

//-------------------------
// Main Loop
//-------------------------
void loop() {
  unsigned long now = millis();
  
  // Update OLED display every updateInterval milliseconds
  if (now - lastDisplayUpdate >= updateInterval) {
    updateDisplay();
    lastDisplayUpdate = now;
  }
  
  // Accept incoming TCP connections and ignore data (for mesh communication)
  WiFiClient client = server.available();
  if (client) {
    client.readStringUntil('\n');  // read and ignore
    client.stop();
  }
  
  delay(10);
}
