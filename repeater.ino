#include <SPI.h>
#include <LoRa.h>

const int ledPin = 13;          // LED pin for status indication
const int rssiThreshold = -90;  // RSSI threshold to decide on repeating data
const unsigned long CAD_INTERVAL = 5000; // CAD check interval (milliseconds)

unsigned long lastMessageId = 0;  // Track the last message ID received
unsigned long lastCADCheck = 0;   // Track the last CAD check time

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(9600);
  while (!Serial);

  Serial.println("Setting up LoRa...");

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }
  
  Serial.println("LoRa init succeeded.");

  // Set SPI CS and reset pins

  Serial.println("Setup complete. Listening for packets...");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    int rssi = LoRa.packetRssi(); // Get RSSI of received packet
    Serial.print("RSSI: ");
    Serial.println(rssi);

    unsigned long messageId = 0;
    LoRa.readBytes((uint8_t *)&messageId, sizeof(messageId)); // Read the message ID

    if (messageId != lastMessageId && rssi < rssiThreshold) {
      lastMessageId = messageId; // Update the last message ID received

      Serial.println("Checking CAD...");

      if (checkCAD()) { // Check Channel Activity Detection
        Serial.println("CAD check passed. Forwarding packet...");

        // Forward the message ID and the rest of the packet
        forwardPacket(messageId, packetSize);
      } else {
        Serial.println("CAD check failed. Not forwarding.");
      }
    }
  }
}

bool checkCAD() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastCADCheck >= CAD_INTERVAL) {
    lastCADCheck = currentMillis;
    int16_t rssiValue = LoRa.rssi(); // Read RSSI without blocking
    Serial.print("CAD RSSI: ");
    Serial.println(rssiValue);
    return (rssiValue < rssiThreshold); // Return CAD status based on RSSI
  }
  return false;
}

void forwardPacket(unsigned long messageId, int packetSize) {
  Serial.println("Forwarding packet...");

  // Prepare a buffer to hold the entire packet data
  uint8_t packetBuffer[255]; // Assuming a maximum packet size of 255 bytes

  // Copy the message ID into the packet buffer
  memcpy(packetBuffer, &messageId, sizeof(messageId));

  // Read the rest of the packet and append it to the buffer
  int bufferIndex = sizeof(messageId);
  while (LoRa.available()) {
    packetBuffer[bufferIndex++] = LoRa.read();
  }

  // Forward the packet buffer contents
  LoRa.beginPacket();
  LoRa.write(packetBuffer, bufferIndex);
  LoRa.endPacket();

  digitalWrite(ledPin, HIGH); // Indicate transmission
  delay(1000);
  digitalWrite(ledPin, LOW);

  Serial.println("Packet forwarded.");
}
