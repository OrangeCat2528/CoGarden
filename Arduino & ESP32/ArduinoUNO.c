#include "DHT.h"
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <LoRa.h>

#define DHTPIN 7 
#define DHTTYPE DHT22 // DHT 22 (AM2302)

#define LORA_NSS 10
#define LORA_RESET 9
#define LORA_DIO0 3

#define LORA_FREQ 915E6
#define LORA_TX_POWER 23 // Daya transmisi LoRa (dBm)
#define LORA_BANDWIDTH 125E3 // Bandwidth LoRa (Hz)
#define LORA_SPREADING_FACTOR 8 // Spreading Factor LoRa

#define ENDPOINT "http://10.10.4.115:3000/cogarden"

const int soilDry = 239;
const int soilWet = 595;

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Inisialisasi LCD dengan alamat 0x3F dan dimensi 16x2

unsigned long previousMillis = 0; // Variabel untuk menyimpan waktu sebelumnya
int displayMode = 0; // Variabel untuk melacak kondisi yang akan ditampilkan

float humidity, temperature;
int soilHumidityPercentage;

bool loraConnected = false;
unsigned long loraLastSent = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();

  // Periksa koneksi pin LoRa
  checkLoRaPinConnection();

  if (loraConnected) {
    Serial.println("Koneksi LoRa berhasil!");
  } else {
    Serial.println("Koneksi LoRa gagal!");
  }

  dht.begin();
}

void loop() {
  unsigned long currentMillis = millis(); // Waktu saat ini dalam milidetik

  if (currentMillis - previousMillis >= 2000) { // Jika sudah 2 detik berlalu
    previousMillis = currentMillis; // Perbarui waktu sebelumnya
    displayMode = (displayMode + 1) % 3; // Ubah kondisi yang akan ditampilkan

    // Baca data dari sensor
    humidity = round(dht.readHumidity());
    temperature = round(dht.readTemperature());
    int soilHumidity = analogRead(A0);
    soilHumidityPercentage = map(soilHumidity, soilWet, soilDry, 0, 100);
    soilHumidityPercentage = constrain(soilHumidityPercentage, 0, 100);

    // Kirim data melalui LoRa jika terhubung
    if (loraConnected && currentMillis - loraLastSent >= 100) {
      loraLastSent = currentMillis;
      sendLoRaMessage(ENDPOINT "?h=" + String((int)humidity) + "&sh=" + String((int)soilHumidityPercentage) + "&t=" + String((int)temperature));
    }
  }

  lcd.setCursor((16 - 8) / 2, 0); // Memusatkan teks di baris pertama
  lcd.print("CoGarden");

  lcd.setCursor(0, 1); // Pindah ke baris kedua, kolom pertama
  lcd.print("                "); // Menghapus baris kedua

switch (displayMode) {
    case 0: // Tampilkan kelembaban
      lcd.setCursor((16 - 13) / 2, 1); // Memusatkan teks di baris kedua
      lcd.print("Humidity: ");
      lcd.print(humidity);
      lcd.print("%");
      break;
    case 1: // Tampilkan suhu
      lcd.setCursor((19 - 15) / 2, 1); // Memusatkan teks di baris kedua
      lcd.print("Temp: ");
      lcd.print(temperature);
      lcd.write(0xDF); // Karakter derajat Celsius
      lcd.print("C");
      break;
    case 2: // Tampilkan kelembaban tanah
      lcd.setCursor((24 - 16) / 2, 1); // Memusatkan teks di baris kedua
      lcd.print("Soil: ");
      lcd.print(soilHumidityPercentage);
      lcd.print("%");
      break;
  }

  delay(200); // Penundaan 200 milidetik
}

void checkLoRaPinConnection() {
  // Periksa koneksi pin NSS
  pinMode(LORA_NSS, OUTPUT);
  digitalWrite(LORA_NSS, HIGH);
  if (digitalRead(LORA_NSS) != HIGH) {
    Serial.println("Koneksi pin NSS LoRa tidak valid!");
    return;
  }

  // Periksa koneksi pin RESET
  pinMode(LORA_RESET, OUTPUT);
  digitalWrite(LORA_RESET, LOW);
  delay(10);
  digitalWrite(LORA_RESET, HIGH);
  if (digitalRead(LORA_RESET) != HIGH) {
    Serial.println("Koneksi pin RESET LoRa tidak valid!");
    return;
  }

  // Periksa koneksi pin DIO0
  pinMode(LORA_DIO0, INPUT);
  if (digitalRead(LORA_DIO0) != LOW) {
    Serial.println("Koneksi pin DIO0 LoRa tidak valid!");
    return;
  }

  // Inisialisasi LoRa
  LoRa.setPins(LORA_NSS, LORA_RESET, LORA_DIO0);
  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("Inisialisasi LoRa gagal!");
    return;
  }

  loraConnected = true;
}

void sendLoRaMessage(String message) {
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  Serial.println("Pesan terkirim: " + message);
}