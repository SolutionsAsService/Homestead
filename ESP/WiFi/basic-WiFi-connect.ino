#include <WiFi.h>  // Use <ESP8266WiFi.h> if using an ESP8266 board

// Replace with your network credentials
const char* ssid = "emeraldcity";
const char* password = "spacemonkeys";

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println();
  Serial.print("Connected! IP Address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(100); // Small delay for serial monitor readiness
  initWiFi();
}

void loop() {
  // Get the current RSSI (signal strength)
  long rssi = WiFi.RSSI();
  
  // Print the RSSI value to the serial monitor
  Serial.print("RSSI: ");
  Serial.println(rssi);
  
  // Wait for 1 second
  delay(1000);
}
