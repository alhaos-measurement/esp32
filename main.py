import json
from time import sleep

import urequests
from machine import I2C, Pin
from network import STA_IF, WLAN

import bmp280

sleep(5)

SDA_PIN = 4
SCL_PIN = 5
SENSOR_ADDR = 0x77

SSID = "vpn-wifi"
PASSWORD = "kuk2Zumba!"

URL = "http://pressure.pyah.online:8080/api/measure"

# Выключаем пины
Pin(12, Pin.OUT).off()
Pin(13, Pin.OUT).off()

# Подключение к Wi-Fi
wlan = WLAN(STA_IF)
wlan.active(True)
wlan.connect(SSID, PASSWORD)

# Ждем подключения
max_wait = 30
while max_wait > 0:
    if wlan.isconnected():
        print("WiFi connected!")
        break
    print("Waiting for connection...")
    sleep(1)
    max_wait -= 1

if not wlan.isconnected():
    print("Failed to connect to WiFi")
    raise Exception("WiFi connection failed")

print("Network config:", wlan.ifconfig())

i2c = I2C(scl=Pin(SCL_PIN), sda=Pin(SDA_PIN), freq=10000)
bmp = bmp280.BMP280(i2c, SENSOR_ADDR)

while True:
    try:
        data = {
            "sensorID": 1,
            "measureTypeID": 1,
            "unitID": 2,
            "value": round(bmp.pressure * 0.00750062),
        }

        stringData = json.dumps(data)
        print("Sending:", stringData)

        headers = {"Content-Type": "application/json"}
        response = urequests.post(URL, data=stringData, headers=headers)

        print("Response status:", response.status_code)
        print("Response text:", response.text)

        response.close()

    except Exception as e:
        print("Error:", e)
        print("Type of error:", type(e))

    sleep(5)
