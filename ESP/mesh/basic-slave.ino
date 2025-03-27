/******************************
 * Slave Node (Node B)
 * 
 * This ESP32 code connects to WiFi with a static IP and runs a TCP server on port 5000.
 * When a connection is made (by the master), it sends a simple greeting message.
 ******************************/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

//-------------------------
// WiFi & Network Settings
//-------------------------
const char* ssid = "emeraldcity";
const char* password = "spacemonkeys";

// Set a static IP for the slave node (Node B)
IPAddress local_IP(192,168,1,101);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WiFiServer server(5000);

//-------------------------
// Setup
//-------------------------
void setup() {
  Serial.begin(115200);
  delay(100);

  // Configure static IP and connect to WiFi
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

  // Start the TCP server
  server.begin();
  Serial.println("TCP server started on port 5000");
}

//-------------------------
// Main Loop
//-------------------------
void loop() {
  // Check for an incoming client connection
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected.");
    delay(10); // Small delay to allow connection stabilization

    // Send a greeting message to the connected client (master)
    client.println("Hello from Node B");

    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
  }
  delay(10);  // Brief delay to ease CPU load
}
