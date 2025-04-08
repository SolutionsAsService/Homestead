#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>           // For JSON parsing
#include <Adafruit_SSD1306.h>      // For OLED display
#include <Adafruit_GFX.h>          // For graphics routines

// WiFi & API Configuration
const char* ssid = "emeraldcity";
const char* password = "spacemonkeys";
String openWeatherMapApiKey = "Open Weather API Key";
String city = "Taos"; 
String countryCode = "US";

// OLED Display Configuration for a 128x32 display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
// For I2C, address is often 0x3C; pass -1 if no hardware reset pin is used.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Timing for weather updates (milliseconds)
unsigned long lastTime = 0;
unsigned long timerDelay = 10000; // Update every 10 seconds

void setup() {
  Serial.begin(115200);
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED allocation failed");
    while (1); // Loop forever if OLED init fails
  }
  display.clearDisplay();
  display.display();

  WiFiConnect();
}

void loop() {
  // If WiFi drops, attempt reconnection.
  if (WiFi.status() != WL_CONNECTED) {
    WiFiConnect();
  }

  // Update weather every timerDelay milliseconds.
  if ((millis() - lastTime) > timerDelay) {
    UpdateWeatherDisplay();
    lastTime = millis();
  }
}

void WiFiConnect() {
  Serial.println("Connecting to WiFi...");

  // Clear display and show initial status.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting to WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();
  unsigned long elapsed = 0;

  // Attempt connection for up to 20 seconds.
  while (WiFi.status() != WL_CONNECTED && (elapsed < 20000)) {
    delay(500);
    elapsed = millis() - startAttemptTime;
    
    // Print debugging info to Serial.
    Serial.print("Elapsed: ");
    Serial.print(elapsed);
    Serial.print(" ms, WiFi Status: ");
    Serial.println(WiFi.status());
    
    // Update the OLED display with elapsed time and status code.
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Connecting to WiFi...");
    display.setCursor(0, 10);
    display.print("Time: ");
    display.print(elapsed);
    display.println(" ms");
    display.setCursor(0, 20);
    display.print("Status: ");
    display.println(WiFi.status());
    display.display();
  }

  // If still not connected after the timeout, restart the board.
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi Connection Failed. Restarting...");
    ESP.restart();
  } else {
    Serial.print("\nWiFi Connected in ");
    Serial.print(elapsed);
    Serial.print(" ms, Status: ");
    Serial.println(WiFi.status());

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WiFi Connected!");
    display.setCursor(0, 10);
    display.print("Time: ");
    display.print(elapsed);
    display.println(" ms");
    display.display();
    delay(1000);
  }
}

String httpGETRequest(const char* /*unused*/) {
  WiFiClient client;
  HTTPClient http;

  // Build the full URL for the OpenWeatherMap API request.
  String URL = "http://api.openweathermap.org/data/2.5/weather";
  URL += "?q=" + city + "," + countryCode;
  URL += "&units=metric";
  URL += "&APPID=" + openWeatherMapApiKey;
  
  http.begin(client, URL);
  int httpResponseCode = http.GET();
  String payload = "{}";
  
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("API Error Occurred");
    display.display();
  }
  http.end();
  return payload;
}

void UpdateWeatherDisplay() {
  // Retrieve weather data from the API.
  String serverURL = "http://api.openweathermap.org/data/2.5/weather";
  String response = httpGETRequest(serverURL.c_str());

  // Allocate a JSON document; adjust capacity as needed.
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, response);

  if (!error) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Line 0: Header with the city.
    display.setCursor(0, 0);
    display.print(city);
    display.println(" Weather");

    // Retrieve weather data from JSON.
    float temp = doc["main"]["temp"];
    int humidity = doc["main"]["humidity"];
    const char* weatherDesc = doc["weather"][0]["description"];

    // Line 1: Temperature.
    display.setCursor(0, 8);
    display.print("T: ");
    display.print(temp);
    display.println(" C");

    // Line 2: Humidity.
    display.setCursor(0, 16);
    display.print("H: ");
    display.print(humidity);
    display.println("%");

    // Line 3: Conditions (may be truncated if too long).
    display.setCursor(0, 24);
    display.print("C: ");
    display.println(weatherDesc);

    display.display();
  } else {
    Serial.println("Error parsing JSON. Please check the response.");
  }
}
