/******************************
 * Master Node A – RFID-Triggered Network Check & DHT Update
 *
 * OLED Wiring (I²C 4-pin):
 *   VCC -> 3.3V (or 5V if supported)
 *   GND -> GND
 *   SDA -> GPIO21
 *   SCL -> GPIO22
 *
 * RFID Module (RC522) Wiring:
 *   VCC  -> 3.3V
 *   GND  -> GND
 *   SDA  -> GPIO16    (CS)
 *   SCK  -> GPIO18
 *   MOSI -> GPIO23
 *   MISO -> GPIO19
 *   RST  -> GPIO17
 ******************************/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522.h>

//-------------------------
// WiFi & Network Settings
//-------------------------
const char* ssid = "emeraldcity";
const char* password = "spacemonkeys";

// Master static IP
IPAddress local_IP(192,168,1,100);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

// Define peer nodes (change or add as needed)
IPAddress peers[] = {
  IPAddress(192,168,1,101),
  IPAddress(192,168,1,102)
};
const int numPeers = sizeof(peers) / sizeof(peers[0]);

WiFiServer server(5000);

//-------------------------
// OLED Setup (I2C on GPIO21/22)
//-------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32  // Using a 32-pixel height OLED
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//-------------------------
// RFID Setup (RC522)
//-------------------------
#define RST_PIN 17   // RC522 reset pin
#define SS_PIN  16   // RC522 chip select (SDA) pin
MFRC522 mfrc522(SS_PIN, RST_PIN);

//-------------------------
// DHT (Transaction) Storage
//-------------------------
struct Transaction {
  String key;
  String value;
};
const int MAX_TX = 20;
Transaction transactions[MAX_TX];
int txCount = 0;

//-------------------------
// Utility Functions
//-------------------------

// Generate a random 5-character alphanumeric block
String generateRandomBlock() {
  String block = "";
  const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  for (int i = 0; i < 5; i++) {
    int index = random(0, sizeof(charset) - 1);
    block += charset[index];
  }
  return block;
}

// Add a transaction to the DHT array
void addTransaction(String key, String value) {
  if (txCount < MAX_TX) {
    transactions[txCount].key = key;
    transactions[txCount].value = value;
    txCount++;
  } else {
    // If full, shift entries to make room for the new one.
    for (int i = 1; i < MAX_TX; i++) {
      transactions[i - 1] = transactions[i];
    }
    transactions[MAX_TX - 1].key = key;
    transactions[MAX_TX - 1].value = value;
  }
}

// Return a comma-separated list of transactions
String getTransactionsString() {
  String s = "";
  for (int i = 0; i < txCount; i++) {
    if (s != "") s += ",";
    s += transactions[i].key + ":" + transactions[i].value;
  }
  return s;
}

// Check connectivity to peer nodes; returns number of peers that respond
int checkConnectedPeers() {
  int count = 0;
  WiFiClient client;
  for (int i = 0; i < numPeers; i++) {
    if (client.connect(peers[i], 5000)) {
      count++;
      client.stop();
      delay(20); // brief pause between connections
    }
  }
  return count;
}

// Update the OLED with an idle message showing node count and "RFID ready"
void updateIdleDisplay() {
  int connectedPeers = checkConnectedPeers();
  int totalNodes = 1 + connectedPeers;  // Count self plus peers
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Nodes: ");
  display.println(totalNodes);
  display.setTextSize(1);
  display.println("RFID ready");
  display.display();
}

// Display a temporary message on the OLED
void showTempDisplay(String msg, unsigned long duration_ms) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(msg);
  display.display();
  delay(duration_ms);
  updateIdleDisplay();
}

//-------------------------
// Setup
//-------------------------
void setup() {
  Serial.begin(115200);
  delay(100);
  randomSeed(analogRead(34));  // Seed random generator

  // Initialize I2C for OLED on GPIO21 (SDA) and GPIO22 (SCL)
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (true);
  }
  updateIdleDisplay();

  // Initialize RFID (SPI)
  SPI.begin(18, 19, 23, SS_PIN);
  mfrc522.PCD_Init();
  Serial.println("RFID initialized.");

  // Setup WiFi with static IP and connect
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected. IP: " + WiFi.localIP().toString());

  // Start TCP server (for peers)
  server.begin();
  Serial.println("TCP server started on port 5000");
}

//-------------------------
// Main Loop
//-------------------------
void loop() {
  // When an RFID card is scanned...
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    unsigned long now = millis();
    // Generate a random block and create a transaction
    String block = generateRandomBlock();
    String txKey = "TX_" + block;
    String txValue = "T" + String(now);
    addTransaction(txKey, txValue);
    Serial.println("New TX: " + txKey + " " + txValue);
    
    // Check for connected peers (re-check network)
    int connectedPeers = checkConnectedPeers();
    int totalNodes = 1 + connectedPeers;
    
    // Optionally, forward the transaction to all peers
    for (int i = 0; i < numPeers; i++) {
      WiFiClient client;
      if (client.connect(peers[i], 5000)) {
        String cmd = "PUTTX " + txKey + " " + txValue;
        client.println(cmd);
        client.stop();
        Serial.println("Forwarded to peer " + peers[i].toString());
      }
    }
    
    // Build a display message
    String msg = "New Block:\n" + txKey + "\n" + txValue + "\nNodes: " + String(totalNodes);
    showTempDisplay(msg, 3000);
    
    // Halt RFID to avoid duplicate reads
    mfrc522.PICC_HaltA();
  }

  // Process any incoming TCP connections (e.g., GETTX commands)
  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\n');
    req.trim();
    Serial.println("TCP Req: " + req);
    if (req.startsWith("GETTX")) {
      client.println(getTransactionsString());
    } else {
      client.println("Unknown cmd");
    }
    client.stop();
  }
  
  // Update idle display periodically (every 5 seconds) when no RFID event
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000) {
    updateIdleDisplay();
    lastUpdate = millis();
  }
  
  delay(10);
}
