#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "WiFi.h"
#include "Wire.h"
#include "Adafruit_Sensor.h"
#include "Adafruit_BMP280.h"

#define LED_PIN 2
#define DELAY_TIME 5000
#define SSID "alhaos-with-vpn-wifi"
#define PASSWORD "kuk2Zumba!"
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BMP280 bmp;

// standby set standby mode if initializaton failes
void standby(String reason)
{
  int counter = 0;
  while (1)
  {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    counter++;
    if (counter > 15)
    {
    }
    counter = 0;
    Serial.print(reason);
  }
}

// initLed
void initLed()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

// InitSerial
void initSerial()
{
  Serial.begin(115200);
  delay(2000);
}

// initBMP280 BMP280 sensor initialization
void initBMP280()
{
  if (!bmp.begin(0x76))
  {
    standby("init initBMP280: failed");
  }
  bmp.setSampling(
      Adafruit_BMP280::MODE_NORMAL,
      Adafruit_BMP280::SAMPLING_X2,
      Adafruit_BMP280::SAMPLING_X16,
      Adafruit_BMP280::FILTER_X16,
      Adafruit_BMP280::STANDBY_MS_500);
  Serial.println("init initBMP280: success");
}

// initWifi Wifi adapter initialization
void initWifi()
{
  WiFi.begin(SSID, PASSWORD);
  int counter = 0;

  while (1)
  {

    delay(1000);

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("init WiFi: success");
      return;
    }
    if (counter > 60)
    {
      standby("init WiFi: failes");
    }
  }
}

void setup()
{
  initLed();
  initSerial();
  initBMP280();
  initWifi();

  Serial.println("Init complete");
}

void loop()
{

  float pressure_mmhg = bmp.readPressure() * 0.00750062;

  JsonDocument doc;

  doc["sensorID"] = 1;
  doc["measureTypeID"] = 1;
  doc["unitID"] = 1;
  doc["value"] = pressure_mmhg;

  String jsonData;
  serializeJson(doc, jsonData);

  WiFiClient client;
  HTTPClient http;

  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(10000);

  http.begin(client, "http://pressure.pyah.online:8080/api/measure/");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Connection", "close");

  int httpCode = http.POST(jsonData);
  Serial.println("Sent: " + jsonData);
  Serial.println("HTTP Response: " + String(httpCode));

  delay(DELAY_TIME);
}