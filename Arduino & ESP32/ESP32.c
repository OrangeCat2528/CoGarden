#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>

// define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

void setup() {
  // initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");

  // setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  // replace the LoRa.begin(---E-) argument with your location's frequency 
  // 433E6 for Asia
  // 866E6 for Europe
  // 915E6 for North America
  while (!LoRa.begin(915E6)) {
    Serial.println("LoRa initialization failed! Check your connections and restart.");
    delay(5000); // wait for 5 seconds before retrying
  }
  
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  // connect to WiFi
  WiFi.begin("Thursina Hotspot");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  static unsigned long lastPacketTime = 0; // variable to store the last time a packet was received
  static bool loraConnected = true; // flag to track LoRa connectivity status
  static bool wifiConnected = true; // flag to track WiFi connectivity status

  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    String url = "";
    while (LoRa.available()) {
      url += (char)LoRa.read();
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());

    // send data to web server
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(url);
      int httpResponseCode = http.GET();
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println("Received payload: " + payload);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    } else {
      wifiConnected = false;
    }

    lastPacketTime = millis(); // update the last packet time
  }

  // check if it's been one second since the last packet
  if (millis() - lastPacketTime >= 100) {
    // check LoRa connectivity
    if (LoRa.beginPacket()) {
      loraConnected = true;
      LoRa.endPacket();
    } else {
      loraConnected = false;
    }
    
    // print "Waiting for packet..." only if LoRa and WiFi are connected
    if (loraConnected && wifiConnected) {
      Serial.println("Waiting for packet...");
    }
    
    lastPacketTime = millis(); // update the last packet time
  }
}