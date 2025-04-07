/*
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files. The above copyright notice
  and this permission notice shall be included in all copies or substantial portions
  of the Software.
  Example adapted from: https://github.com/mobizt/ESP-Mail-Client
*/

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <ESP_Mail_Client.h>

// Wi-Fi credentials
#define WIFI_SSID       "emeraldcity"
#define WIFI_PASSWORD   "spacemonkeys"

// SMTP server configuration
#define SMTP_HOST       "smtp.gmail.com"
#define SMTP_PORT       465

/* The sign in credentials */
#define AUTHOR_EMAIL "email"
#define AUTHOR_PASSWORD "pass"

/* Recipient's email*/
#define RECIPIENT_EMAIL "recep"

// Global SMTPSession object for email transport
SMTPSession smtp;

// Timing settings: 30-second email interval and a 15-second Wi-Fi timeout
unsigned long lastEmailTime = 0;
const unsigned long emailInterval = 30000; // 30,000 milliseconds
const unsigned long wifiTimeout   = 15000; // 15,000 milliseconds

// Callback to report SMTP status via Serial
void smtpCallback(SMTP_Status status);

// Connect to Wi-Fi (only prints status changes, no repeated dot printing)
bool connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED)
    return true;

  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > wifiTimeout) {
      Serial.println("Wi-Fi connection timed out.");
      return false;
    }
    delay(300);
  }
  Serial.print("Wi-Fi connected. IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

// Configure and send the email message
void sendEmail() {
  // Configure the SMTP session
  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";
  
  // Set NTP time (adjust offsets as needed)
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  // Prepare the email message
  SMTP_Message message;
  message.sender.name = F("ESP");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = F("i h8 asians");
  message.addRecipient(F("Retard"), RECIPIENT_EMAIL);

  String textMsg = "PAY ME NIGGA - sent from ESP board";
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | 
                              esp_mail_smtp_notify_failure | 
                              esp_mail_smtp_notify_delay;

  // Connect to the SMTP server
  if (!smtp.connect(&config)) {
    Serial.printf("SMTP connection error: %d, %d, %s\n",
                  smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (smtp.isLoggedIn()) {
    Serial.println("SMTP logged in.");
  } else {
    Serial.println("SMTP connected (no auth).");
  }

  // Send the email
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.printf("Error sending email: %d, %d, %s\n",
                  smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  } else {
    Serial.println("Email sent successfully.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Give a moment for system boot messages to finish

  // Initial Wi-Fi connection (attempt once)
  if (!connectToWiFi()) {
    Serial.println("Initial Wi-Fi connection failed. Restarting...");
    ESP.restart();
  }
  
  // Enable automatic network reconnection for email
  MailClient.networkReconnect(true);

  // Enable debugging and set the SMTP callback
  smtp.debug(1);
  smtp.callback(smtpCallback);

  lastEmailTime = millis();
}

void loop() {
  // If Wi-Fi is lost, try reconnecting (only reports once per disconnection)
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Attempting to reconnect...");
    connectToWiFi();
  }
  
  unsigned long currentMillis = millis();
  if (currentMillis - lastEmailTime >= emailInterval) {
    sendEmail();
    lastEmailTime = currentMillis;
  }
}

// Callback function for SMTP status reporting
void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());
  if (status.success()) {
    Serial.println("---------- Email Send Summary ----------");
    Serial.printf("Sent: %d  Failed: %d\n", status.completedCount(), status.failedCount());
    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      SMTP_Result result = smtp.sendingResult.getItem(i);
      Serial.printf("Message %d - Status: %s, Recipient: %s, Subject: %s\n", 
                    i + 1, result.completed ? "Success" : "Failed",
                    result.recipients.c_str(), result.subject.c_str());
    }
    Serial.println("----------------------------------------\n");
    smtp.sendingResult.clear();
  }
}
