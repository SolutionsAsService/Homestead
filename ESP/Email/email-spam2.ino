/*
  ESP32 OLED Email Test Bot
  -------------------------

  Uses:
    - ESP32
    - SSD1306 128x64 OLED
    - WiFi
    - ESP-Mail-Client

  OLED wiring:

      SSD1306       ESP32
      --------------------
      VCC     ->     3.3V
      GND     ->     GND
      SDA     ->     GPIO 21
      SCL     ->     GPIO 22

  OLED I2C address:
      0x3C

  IMPORTANT:
    Use an App Password for Gmail rather than your normal
    Gmail account password.

  The send interval is controlled by:

      #define EMAIL_INTERVAL_SECONDS 30

  Change that number to whatever interval you want for
  your personal testing.
*/

#include <Arduino.h>

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP_Mail_Client.h>


// ============================================================
// OLED CONFIGURATION
// ============================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 21
#define OLED_SCL 22

#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);


// ============================================================
// WIFI CONFIGURATION
// ============================================================

#define WIFI_SSID       "emeraldcity"
#define WIFI_PASSWORD   "spacemonkeys"


// ============================================================
// SMTP CONFIGURATION
// ============================================================

#define SMTP_HOST       "smtp.gmail.com"
#define SMTP_PORT       465

#define AUTHOR_EMAIL    "author email"
#define AUTHOR_PASSWORD "generated password"

#define RECIPIENT_EMAIL "recep"


// ============================================================
// EASY RATE LIMIT
// ============================================================
//
// Change ONLY this number to change the interval.
//
// Examples:
//
//   10  = every 10 seconds
//   30  = every 30 seconds
//   60  = every minute
//   300 = every 5 minutes
//   600 = every 10 minutes
//
// ============================================================

#define EMAIL_INTERVAL_SECONDS 30


// Convert seconds to milliseconds
const unsigned long EMAIL_INTERVAL =
  (unsigned long)EMAIL_INTERVAL_SECONDS * 1000UL;


// WiFi timeout
const unsigned long WIFI_TIMEOUT = 15000;


// OLED refresh interval
const unsigned long OLED_UPDATE_INTERVAL = 500;


// ============================================================
// SMTP OBJECT
// ============================================================

SMTPSession smtp;


// ============================================================
// GLOBAL STATE
// ============================================================

unsigned long lastEmailTime = 0;
unsigned long lastOLEDUpdate = 0;

unsigned long emailsSent = 0;
unsigned long emailsFailed = 0;

String currentStatus = "BOOTING";
String lastError = "";


// ============================================================
// FUNCTION DECLARATIONS
// ============================================================

void smtpCallback(SMTP_Status status);

bool connectToWiFi();

void sendEmail();

void updateOLED();

void showBootScreen();


// ============================================================
// OLED BOOT SCREEN
// ============================================================

void showBootScreen()
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(10, 5);
  display.println("EMAIL");

  display.setCursor(10, 27);
  display.println("BOT");

  display.setTextSize(1);

  display.setCursor(10, 50);
  display.println("Starting...");

  display.display();
}


// ============================================================
// OLED STATUS SCREEN
// ============================================================

void updateOLED()
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  // ----------------------------------------------------------
  // HEADER
  // ----------------------------------------------------------

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("ESP EMAIL BOT");

  display.drawLine(
    0,
    9,
    127,
    9,
    SSD1306_WHITE
  );


  // ----------------------------------------------------------
  // WIFI STATUS
  // ----------------------------------------------------------

  display.setCursor(0, 13);
  display.print("WiFi: ");

  if (WiFi.status() == WL_CONNECTED)
  {
    display.println("OK");
  }
  else
  {
    display.println("OFF");
  }


  // ----------------------------------------------------------
  // IP ADDRESS
  // ----------------------------------------------------------

  if (WiFi.status() == WL_CONNECTED)
  {
    display.setCursor(0, 23);

    display.print(
      WiFi.localIP().toString()
    );
  }


  // ----------------------------------------------------------
  // STATUS
  // ----------------------------------------------------------

  display.setCursor(0, 34);

  display.print("STATUS: ");

  String status = currentStatus;

  if (status.length() > 13)
  {
    status = status.substring(0, 13);
  }

  display.println(status);


  // ----------------------------------------------------------
  // SEND COUNTER
  // ----------------------------------------------------------

  display.setCursor(0, 44);

  display.print("SENT: ");
  display.print(emailsSent);

  display.print("  ERR: ");
  display.print(emailsFailed);


  // ----------------------------------------------------------
  // COUNTDOWN
  // ----------------------------------------------------------

  unsigned long now = millis();

  unsigned long elapsed =
    now - lastEmailTime;

  unsigned long remaining = 0;

  if (elapsed < EMAIL_INTERVAL)
  {
    remaining =
      (EMAIL_INTERVAL - elapsed) / 1000;
  }

  display.setCursor(0, 55);

  display.print("NEXT: ");

  display.print(remaining);

  display.print("s");


  // ----------------------------------------------------------
  // DISPLAY
  // ----------------------------------------------------------

  display.display();
}


// ============================================================
// WIFI CONNECTION
// ============================================================

bool connectToWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return true;
  }

  currentStatus = "WIFI...";

  updateOLED();

  Serial.println();
  Serial.println("Connecting to WiFi...");

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  unsigned long startAttemptTime =
    millis();


  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - startAttemptTime < WIFI_TIMEOUT
  )
  {
    delay(300);

    Serial.print(".");
  }

  Serial.println();


  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println(
      "WiFi connected."
    );

    Serial.print(
      "IP address: "
    );

    Serial.println(
      WiFi.localIP()
    );

    currentStatus = "WIFI OK";

    updateOLED();

    return true;
  }


  Serial.println(
    "WiFi connection timed out."
  );

  currentStatus = "WIFI FAIL";

  updateOLED();

  return false;
}


// ============================================================
// SEND EMAIL
// ============================================================

void sendEmail()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(
      "Cannot send email: WiFi offline."
    );

    currentStatus = "NO WIFI";

    return;
  }


  Serial.println();
  Serial.println(
    "Preparing email..."
  );


  currentStatus = "SMTP";

  updateOLED();


  // ----------------------------------------------------------
  // SMTP CONFIG
  // ----------------------------------------------------------

  Session_Config config;

  config.server.host_name =
    SMTP_HOST;

  config.server.port =
    SMTP_PORT;

  config.login.email =
    AUTHOR_EMAIL;

  config.login.password =
    AUTHOR_PASSWORD;

  config.login.user_domain = "";


  // ----------------------------------------------------------
  // TIME CONFIG
  // ----------------------------------------------------------

  config.time.ntp_server =
    F("pool.ntp.org,time.nist.gov");

  config.time.gmt_offset = -6;

  config.time.day_light_offset = 1;


  // ----------------------------------------------------------
  // MESSAGE
  // ----------------------------------------------------------

  SMTP_Message message;

  message.sender.name =
    F("ESP32 Email Bot");

  message.sender.email =
    AUTHOR_EMAIL;

  message.subject =
    F("ESP32 Test Email");

  message.addRecipient(
    F("Friend"),
    RECIPIENT_EMAIL
  );


  // ----------------------------------------------------------
  // EMAIL BODY
  // ----------------------------------------------------------

  String textMsg =
    "Hello from the ESP32!\n\n"
    "This message was generated by "
    "the ESP32 email test bot.\n\n"
    "Send interval: "
    + String(EMAIL_INTERVAL_SECONDS)
    + " seconds\n\n"
    "Messages sent this session: "
    + String(emailsSent + 1);


  message.text.content =
    textMsg.c_str();

  message.text.charSet =
    "us-ascii";

  message.text.transfer_encoding =
    Content_Transfer_Encoding::enc_7bit;


  // ----------------------------------------------------------
  // EMAIL PRIORITY
  // ----------------------------------------------------------

  message.priority =
    esp_mail_smtp_priority::
    esp_mail_smtp_priority_low;


  message.response.notify =
      esp_mail_smtp_notify_success
    | esp_mail_smtp_notify_failure
    | esp_mail_smtp_notify_delay;


  // ----------------------------------------------------------
  // CONNECT SMTP
  // ----------------------------------------------------------

  Serial.println(
    "Connecting to SMTP..."
  );

  if (!smtp.connect(&config))
  {
    Serial.printf(
      "SMTP connection error\n"
    );

    Serial.printf(
      "Status Code: %d\n",
      smtp.statusCode()
    );

    Serial.printf(
      "Error Code: %d\n",
      smtp.errorCode()
    );

    Serial.printf(
      "Reason: %s\n",
      smtp.errorReason().c_str()
    );


    lastError =
      smtp.errorReason();

    currentStatus =
      "SMTP ERROR";

    emailsFailed++;

    updateOLED();

    return;
  }


  smtpConnected = true;


  if (smtp.isLoggedIn())
  {
    Serial.println(
      "SMTP logged in."
    );

    currentStatus =
      "SENDING";
  }
  else
  {
    Serial.println(
      "SMTP connected."
    );

    currentStatus =
      "SENDING";
  }


  updateOLED();


  // ----------------------------------------------------------
  // SEND
  // ----------------------------------------------------------

  Serial.println(
    "Sending email..."
  );


  if (
    !MailClient.sendMail(
      &smtp,
      &message
    )
  )
  {
    Serial.printf(
      "Error sending email\n"
    );

    Serial.printf(
      "Status Code: %d\n",
      smtp.statusCode()
    );

    Serial.printf(
      "Error Code: %d\n",
      smtp.errorCode()
    );

    Serial.printf(
      "Reason: %s\n",
      smtp.errorReason().c_str()
    );


    lastError =
      smtp.errorReason();

    currentStatus =
      "SEND ERROR";

    emailsFailed++;

    updateOLED();

    return;
  }


  // ----------------------------------------------------------
  // SUCCESS
  // ----------------------------------------------------------

  emailsSent++;

  currentStatus =
    "SENT!";

  Serial.println();
  Serial.println(
    "================================"
  );

  Serial.println(
    "EMAIL SENT SUCCESSFULLY"
  );

  Serial.print(
    "Total sent: "
  );

  Serial.println(
    emailsSent
  );

  Serial.println(
    "================================"
  );


  updateOLED();


  // ----------------------------------------------------------
  // CLOSE SMTP SESSION
  // ----------------------------------------------------------

  smtp.closeSession();

  smtpConnected = false;
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  delay(500);


  // ----------------------------------------------------------
  // OLED
  // ----------------------------------------------------------

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );


  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      OLED_ADDRESS
    )
  )
  {
    Serial.println(
      "SSD1306 OLED not found!"
    );

    while (true)
    {
      delay(1000);
    }
  }


  showBootScreen();

  delay(1000);


  // ----------------------------------------------------------
  // WIFI
  // ----------------------------------------------------------

  WiFi.mode(WIFI_STA);

  WiFi.setAutoReconnect(true);

  WiFi.persistent(false);


  if (!connectToWiFi())
  {
    Serial.println(
      "WiFi not connected."
    );

    currentStatus =
      "WIFI FAIL";

    updateOLED();
  }


  // ----------------------------------------------------------
  // SMTP NETWORK RECONNECT
  // ----------------------------------------------------------

  MailClient.networkReconnect(true);


  // ----------------------------------------------------------
  // SMTP DEBUG
  // ----------------------------------------------------------

  smtp.debug(1);

  smtp.callback(
    smtpCallback
  );


  // ----------------------------------------------------------
  // START TIMER
  // ----------------------------------------------------------

  lastEmailTime =
    millis();


  currentStatus =
    "READY";


  updateOLED();


  Serial.println();
  Serial.println(
    "================================"
  );

  Serial.println(
    "ESP32 EMAIL BOT READY"
  );

  Serial.print(
    "Email interval: "
  );

  Serial.print(
    EMAIL_INTERVAL_SECONDS
  );

  Serial.println(
    " seconds"
  );

  Serial.println(
    "================================"
  );
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
  // ----------------------------------------------------------
  // WIFI RECONNECT
  // ----------------------------------------------------------

  if (WiFi.status() != WL_CONNECTED)
  {
    smtpConnected = false;

    currentStatus =
      "RECONNECT";

    Serial.println(
      "WiFi disconnected."
    );

    connectToWiFi();
  }


  // ----------------------------------------------------------
  // EMAIL TIMER
  // ----------------------------------------------------------

  unsigned long currentMillis =
    millis();


  if (
    currentMillis - lastEmailTime
    >= EMAIL_INTERVAL
  )
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      sendEmail();
    }

    lastEmailTime =
      currentMillis;
  }


  // ----------------------------------------------------------
  // OLED UPDATE
  // ----------------------------------------------------------

  if (
    currentMillis - lastOLEDUpdate
    >= OLED_UPDATE_INTERVAL
  )
  {
    updateOLED();

    lastOLEDUpdate =
      currentMillis;
  }


  delay(10);
}


// ============================================================
// SMTP CALLBACK
// ============================================================

void smtpCallback(
  SMTP_Status status
)
{
  Serial.println();

  Serial.println(
    status.info()
  );


  if (status.success())
  {
    Serial.println(
      "----------------"
    );

    Serial.printf(
      "Messages sent: %d\n",
      status.completedCount()
    );

    Serial.printf(
      "Messages failed: %d\n",
      status.failedCount()
    );

    Serial.println(
      "----------------"
    );


    // --------------------------------------------------------
    // PRINT INDIVIDUAL RESULTS
    // --------------------------------------------------------

    for (
      size_t i = 0;
      i < smtp.sendingResult.size();
      i++
    )
    {
      SMTP_Result result =
        smtp.sendingResult.getItem(i);


      Serial.printf(
        "Message %d - Status: %s\n",
        i + 1,
        result.completed
          ? "SUCCESS"
          : "FAILED"
      );


      Serial.printf(
        "Date/Time: %s\n",
        MailClient.Time.getDateTimeString(
          result.timestamp,
          "%B %d, %Y %H:%M:%S"
        ).c_str()
      );


      Serial.printf(
        "Recipient: %s\n",
        result.recipients.c_str()
      );


      Serial.printf(
        "Subject: %s\n",
        result.subject.c_str()
      );
    }


    Serial.println(
      "----------------"
    );


    smtp.sendingResult.clear();
  }
}
