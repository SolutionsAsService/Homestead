/******************************
 * Master Node Display – Show Connected Nodes
 *
 * OLED Wiring (I²C 4-pin):
 *   VCC -> 3.3V (or 5V if supported)
 *   GND -> GND
 *   SDA -> GPIO21
 *   SCL -> GPIO22
 *
 * This program checks connectivity with peer nodes (via TCP on port 5000)
 * and displays the number of connected nodes on the OLED.
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

// Set a static IP for the master ESP32
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

//-------------------------
// TCP Server (for peer communication, if needed)
//-------------------------
WiFiServer server(5000);

//-------------------------
// Timing Variables
//-------------------------
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 5000; // update every 5 seconds

//-------------------------
// Function to check connectivity to peers
//-------------------------
int checkConnectedPeers() {
  int connected = 0;
  WiFiClient client;
  for (int i = 0; i < numPeers; i++) {
    if (client.connect(peers[i], 5000)) {  // try to connect on port 5000
      connected++;
      client.stop();
      delay(50);  // brief pause between connections
    }
  }
  return connected;
}

//-------------------------
// Function to update the OLED display
//-------------------------
void updateDisplay() {
  int peersConnected = checkConnectedPeers();
  // Count self as connected too (so total nodes = 1 + peersConnected)
  int totalNodes = 1 + peersConnected;

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Nodes:");
  display.print(totalNodes);
  display.display();
}

//-------------------------
// Setup
//-------------------------
void setup() {
  Serial.begin(115200);
  delay(100);
  
  // Initialize I2C for OLED on pins 21 (SDA) and 22 (SCL)
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
  Serial.println(WiFi.localIP());
  
  // Start TCP server (if peers need to connect)
  server.begin();
  
  // Initial display update
  updateDisplay();
  lastUpdate = millis();
}

//-------------------------
// Main Loop
//-------------------------
void loop() {
  unsigned long now = millis();
  
  // Update display every updateInterval milliseconds
  if (now - lastUpdate >= updateInterval) {
    updateDisplay();
    lastUpdate = now;
  }
  
  // Process any incoming TCP connections (optional)
  WiFiClient client = server.available();
  if (client) {
    // For now, simply read and ignore any incoming data.
    client.readStringUntil('\n');
    client.stop();
  }
  
  // Small delay to reduce CPU load
  delay(10);
}
