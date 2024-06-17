#include "DHT.h"
#include <LiquidCrystal_I2C.h>
#include "LoRa_E220.h"
#include <SoftwareSerial.h>

#define DHTPIN 7 
#define DHTTYPE DHT22 // DHT 22 (AM2302)

#define LIGHTSENSORPIN A1 // Ambient light sensor reading
#define DESTINATION_ADDL 20

const int soilDry = 239;
const int soilWet = 595;

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Initialize LCD with address 0x27 and dimensions 16x2

SoftwareSerial mySerial(2, 3); // RX, TX
LoRa_E220 e220ttl(&mySerial);

unsigned long previousMillis = 0; // Variable to store previous time for LCD update
unsigned long previousLoRaMillis = 0; // Variable to store previous time for LoRa transmission
int displayMode = 0; // Variable to track which condition to display

float humidity, temperature, lightPercentage;
int soilHumidityPercentage;
bool loraError = false;

void setup() {
  Serial.begin(9600);
  delay(500);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  dht.begin();
  e220ttl.begin();
}

void loop() {
  unsigned long currentMillis = millis(); // Current time in milliseconds

  if (currentMillis - previousMillis >= 2000) { // If 2 seconds have passed for LCD update
    previousMillis = currentMillis; // Update previous time
    displayMode = (displayMode + 1) % 4; // Change condition to display

    // Read data from sensors
    humidity = round(dht.readHumidity());
    temperature = round(dht.readTemperature());
    int soilHumidity = analogRead(A0);
    soilHumidityPercentage = map(soilHumidity, soilWet, soilDry, 0, 100);
    soilHumidityPercentage = constrain(soilHumidityPercentage, 0, 100);
    
    // Read light level and convert to percentage
    float lightReading = analogRead(LIGHTSENSORPIN);
    lightPercentage = (lightReading / 1023.0) * 100.0;

    lcd.setCursor((16 - 8) / 2, 0); // Center text on first line
    lcd.print("CoGarden");

    lcd.setCursor(0, 1); // Move to first column of second line
    lcd.print("                "); // Clear second line

    if (loraError) {
      lcd.setCursor((16 - 9) / 2, 1); // Center "LoRa Error!" on second line
      lcd.print("LoRa Error!");
    } else {
      switch (displayMode) {
        case 0: // Display humidity
          lcd.setCursor((16 - 13) / 2, 1); // Center text on second line
          lcd.print("Humidity: ");
          lcd.print(humidity);
          lcd.print("%");
          break;
        case 1: // Display temperature
          lcd.setCursor((19 - 15) / 2, 1); // Center text on second line
          lcd.print("Temp: ");
          lcd.print(temperature);
          lcd.write(0xDF); // Degree Celsius character
          lcd.print("C");
          break;
        case 2: // Display soil moisture
          lcd.setCursor((24 - 16) / 2, 1); // Center text on second line
          lcd.print("Soil: ");
          lcd.print(soilHumidityPercentage);
          lcd.print("%");
          break;
        case 3: // Display light level
          lcd.setCursor((24 - 16) / 2, 1); // Center text on second line
          lcd.print("Light: ");
          lcd.print(lightPercentage);
          lcd.print("%");
          break;
      }
    }
  }

  if (currentMillis - previousLoRaMillis >= 2000) { // If 2 seconds have passed for LoRa transmission
    previousLoRaMillis = currentMillis; // Update previous time for LoRa

    // Create JSON formatted string
    String jsonOutput = "{\"s\":\"" + String(humidity) + "\",\"sh\":\"" + String(soilHumidityPercentage) + "\",\"t\":\"" + String((int)temperature) + "\",\"light\":\"" + String(lightPercentage) + "\"}";
    Serial.println(jsonOutput);

    // Send JSON string via LoRa
    ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, jsonOutput);
    if (rs.code == 1) {
      Serial.println("Pesan Successfully Sent! (" + jsonOutput + ")");
      loraError = false;
    } else {
      Serial.println("Failed to send message: " + rs.getResponseDescription());
      loraError = true;
    }
  }

  delay(200); // 200 milliseconds delay
}
