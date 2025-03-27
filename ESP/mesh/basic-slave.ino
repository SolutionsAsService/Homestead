/******************************
 * Peer Node – Mesh Connection
 *
 * This sketch connects to WiFi with a static IP,
 * starts a TCP server on port 5000, and waits for 
 * connection requests from the master node.
 *
 * When the master connects, it responds with "OK"
 * and logs the connection in the serial monitor.
 ******************************/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

//-------------------------
// WiFi & Network Settings
//-------------------------
const char* ssid = "emeraldcity";
const char* password = "spacemonkeys";

// Assign a unique static IP for this peer
IPAddress local_IP(192,168,1,101);  // Change this for each peer
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WiFiServer server(5000);

//-------------------------
// Setup
//-------------------------
void setup() {
  Serial.begin(115200);
  delay(100);
  
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
}

//-------------------------
// Main Loop
//-------------------------
void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("Master connected.");
    
    // Send response to master
    client.println("OK");
    
    // Wait briefly before closing connection
    delay(50);
    client.stop();
    Serial.println("Connection closed.");
  }
  
  delay(10);
}
