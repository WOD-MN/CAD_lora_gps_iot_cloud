#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 7, TXPin = 6;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

unsigned long messageId = 0; // Message ID initialization

void setup() {
  Serial.begin(9600);
  ss.begin(GPSBaud);
  while (!Serial);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
}

void loop() {
  smartDelay(5000);

  // If GPS data is available, print latitude and longitude
  if (gps.location.isValid()) {
    messageId++; // Increment message ID for each transmission

    Serial.print("Message ID: ");
    Serial.println(messageId);
    
    Serial.print("Latitude: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", Longitude: ");
    Serial.println(gps.location.lng(), 6);
    
    // Create message string with message ID, latitude, and longitude
    String message = String(messageId) + "," + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
    char dataToSend[50];
    message.toCharArray(dataToSend, 50);
    
    LoRa.beginPacket();
    LoRa.println(dataToSend);
    LoRa.endPacket();
  }
}

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
