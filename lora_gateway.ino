#include "arduino_secrets.h"
#include "thingProperties.h"
#include <SPI.h>
#include <LoRa.h>

#define LORA_SS 15   // LoRa radio chip select (CS) pin
#define LORA_RST 16 // LoRa radio reset (RST) pin
#define LORA_DI0 4  // LoRa radio digital input (DIO0) pin

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
   LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);

  // Initialize LoRa module
 LoRa.begin(433E6);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500); 

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();
  
  if (LoRa.parsePacket()) {
    // Receive data
    while (LoRa.available()) {
      String receivedData = LoRa.readString();
      Serial.println("Received data: " + receivedData);

      int firstCommaIndex = receivedData.indexOf(',');
      int secondCommaIndex = receivedData.indexOf(',', firstCommaIndex + 1);

      if (firstCommaIndex != -1 && secondCommaIndex != -1) {
        // Extract latitude and longitude substrings from the received message
        String latString = receivedData.substring(firstCommaIndex + 1, secondCommaIndex);
        String lonString = receivedData.substring(secondCommaIndex + 1);

        // Convert latitude and longitude strings to float values
        float latitude = latString.toFloat();
        float longitude = lonString.toFloat();

        // Set the CloudLocation variable x with the extracted latitude and longitude
        x = Location(latitude, longitude);

        // Report the values to IoT Cloud
        ArduinoCloud.update();

        // Print values to Serial Monitor
        Serial.print("Latitude: ");
        Serial.println(latitude);
        Serial.print("Longitude: ");
        Serial.println(longitude);
      }
    }
  }
}