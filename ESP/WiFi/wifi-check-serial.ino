#include <WiFi.h>

const char* ssid = "SSID";
const char* password = "pass";

void setup() {
    Serial.begin(115200);
    WiFi.disconnect(true);
    delay(1000);

    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        Serial.print("WiFi Status: ");
        Serial.println(WiFi.status());
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect!");
    }
}


void loop() {}
