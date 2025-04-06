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

// SMTP server details
#define SMTP_HOST       "smtp.gmail.com"
#define SMTP_PORT       465

/* The sign in credentials */
#define AUTHOR_EMAIL    "author email"
#define AUTHOR_PASSWORD "generated password"

/* Recipient's email */
#define RECIPIENT_EMAIL "recep"

SMTPSession smtp;  // Global SMTPSession object

unsigned long lastEmailTime = 0;
const unsigned long emailInterval = 30000; // 30,000 ms = 30 seconds
const unsigned long wifiTimeout   = 15000; // 15,000 ms timeout

// Callback function for SMTP status updates
void smtpCallback(SMTP_Status status);

// Connect to Wi-Fi only if not already connected
bool connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED)
    return true;

  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
    delay(300);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Wi-Fi connected with IP: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("Wi-Fi connection timed out.");
    return false;
  }
}

void sendEmail() {
  // Set up SMTP session configuration
  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  // Set NTP configuration (adjust GMT offset if needed)
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  // Configure email message
  SMTP_Message message;
  message.sender.name = F("ESP");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = F("ESP Test Email");
  message.addRecipient(F("Dummy"), RECIPIENT_EMAIL);

  // Prepare a simple text message
  String textMsg = "Email sent every 30 sec - sent from ESP board";
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | 
                              esp_mail_smtp_notify_failure | 
                              esp_mail_smtp_notify_delay;

  // Connect to SMTP server
  if (!smtp.connect(&config)) {
    Serial.printf("SMTP connection error, Status Code: %d, Error Code: %d, Reason: %s\n",
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
    Serial.printf("Error sending email, Status Code: %d, Error Code: %d, Reason: %s\n",
                  smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  }
}

void setup(){
  Serial.begin(115200);
  Serial.println();

  // Initial Wi-Fi connection attempt
  if (!connectToWiFi()) {
    Serial.println("Wi-Fi not connected. Restart the board to retry.");
    while (true) { delay(1000); }
  }

  // Enable automatic network reconnection for SMTP
  MailClient.networkReconnect(true);

  // Enable SMTP debugging and set callback
  smtp.debug(1);
  smtp.callback(smtpCallback);

  lastEmailTime = millis();
}

void loop(){
  // Check if Wi-Fi is disconnected and try to reconnect
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Attempting to reconnect...");
    connectToWiFi();
  }
  
  // Send email every 30 seconds if connected
  unsigned long currentMillis = millis();
  if (currentMillis - lastEmailTime >= emailInterval) {
    sendEmail();
    lastEmailTime = currentMillis;
  }
}

// SMTP status callback function
void smtpCallback(SMTP_Status status){
  Serial.println(status.info());
  if (status.success()){
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    // Show details for each result
    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      SMTP_Result result = smtp.sendingResult.getItem(i);
      Serial.printf("Message %d - Status: %s\n", i + 1, result.completed ? "success" : "failed");
      Serial.printf("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      Serial.printf("Recipient: %s\n", result.recipients.c_str());
      Serial.printf("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");
    smtp.sendingResult.clear();
  }
}
