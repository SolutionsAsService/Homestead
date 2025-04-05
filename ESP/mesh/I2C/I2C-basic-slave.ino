/*********
  Slave code for ESP32 I2C communication.
  
  Note: If you intend to have multiple slaves on the same bus, each must have a unique address.
  
  Required library:
    - Wire.h
*********/

#include <Wire.h>

#define I2C_DEV_ADDR 0x55  // Change this for each slave if you have more than one

uint32_t packetCount = 0;

void onRequest() {
  // When the master requests data, send the current packet count and a message
  Wire.print(packetCount);
  Wire.print(" Packets.");
  Serial.println("onRequest triggered");
  packetCount++;
}

void onReceive(int len) {
  Serial.printf("onReceive[%d]: ", len);
  while (Wire.available()) {
    Serial.write(Wire.read());
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);
  Wire.begin((uint8_t)I2C_DEV_ADDR);
  Serial.printf("Slave started at address 0x%X\n", I2C_DEV_ADDR);
}

void loop() {
  // The slave simply waits for I2C communication from the master.
}
