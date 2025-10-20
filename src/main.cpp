#include <Arduino.h>
#include "WiFi.h"
#include "Wire.h"
#include "Adafruit_Sensor.h"
#include "Adafruit_BMP280.h"

#define LED_PIN 2
#define DELAY_TIME 1000
#define SSID "alhaos-with-vpn-wifi"
#define PASSWORD "kuk2Zumba!"
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BMP280 bmp;

// Функция для сканирования I2C устройств
void scanI2C() {
  Serial.println("🔍 Scanning I2C devices...");
  byte count = 0;
  
  Wire.begin();
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("✅ Found device at: 0x");
      Serial.println(addr, HEX);
      count++;
      delay(10);
    }
  }
  
  if (count == 0) {
    Serial.println("❌ No I2C devices found!");
  } else {
    Serial.println("I2C scan completed.");
  }
}

// Функция инициализации BMP280
bool initBMP280() {
  Serial.println("🔄 Initializing BMP280...");
  
  // Пробуем оба возможных адреса
  if (bmp.begin(0x76)) {
    Serial.println("✅ BMP280 found at address 0x76");
    return true;
  }
  
  if (bmp.begin(0x77)) {
    Serial.println("✅ BMP280 found at address 0x77");
    return true;
  }
  
  Serial.println("❌ BMP280 not found at any address!");
  return false;
}

void setup() {
  // Инициализация пинов
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Инициализация Serial
  Serial.begin(115200);
  delay(2000); // Даем время для подключения монитора порта
  
  Serial.println();
  Serial.println("🚀 Starting ESP32 BMP280 Monitor...");
  
  // Сканируем I2C шину
  scanI2C();
  
  // Инициализируем BMP280
  if (!initBMP280()) {
    Serial.println("❌ Critical: BMP280 initialization failed!");
    Serial.println("🔧 Check: ");
    Serial.println("   - Wiring (SDA/SCL/VCC/GND)");
    Serial.println("   - 3.3V power supply");
    Serial.println("   - I2C address (0x76 or 0x77)");
    
    // Аварийный режим - быстро мигаем
    while (1) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      delay(100);
    }
  }
  
  Serial.println("✅ BMP280 initialized successfully!");
  
  // Настраиваем параметры BMP280
  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,  // temperature
    Adafruit_BMP280::SAMPLING_X16, // pressure
    Adafruit_BMP280::FILTER_X16,
    Adafruit_BMP280::STANDBY_MS_500
  );
  
  // Инициализация WiFi
  Serial.println("📡 Initializing WiFi...");
  Serial.println(String("   SSID: ") + SSID);
  WiFi.begin(SSID, PASSWORD);
  
  // Мигаем пока не подключимся к WiFi
  unsigned long wifiTimeout = 30000; // 30 секунд таймаут
  unsigned long wifiStartTime = millis();
  
  while (WiFi.status() != WL_CONNECTED && 
         millis() - wifiStartTime < wifiTimeout) {
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    delay(250);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("✅ WiFi connected!");
    Serial.print("📶 IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("❌ WiFi connection failed!");
  }
  
  Serial.println("🎉 System ready!");
}

void loop() {
  // Индикация работы
  digitalWrite(LED_PIN, HIGH);
  
  // Чтение данных с BMP280
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F; // Па -> гПа
  float altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  
  // Вывод данных
  Serial.println("=== BMP280 Readings ===");
  Serial.print("🌡️ Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  Serial.print("📊 Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");
  
  Serial.print("⛰️ Approx. Altitude: ");
  Serial.print(altitude);
  Serial.println(" m");
  
  Serial.print("📶 WiFi: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  Serial.println("========================");
  
  digitalWrite(LED_PIN, LOW);
  
  // Задержка между измерениями
  delay(DELAY_TIME);
}