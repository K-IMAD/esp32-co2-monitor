## 🌍 ESP32 CO₂ IoT Monitor

      An IoT-enabled air quality monitoring system using an ESP32, two MH-Z19 CO₂ sensors, and an I²C LCD.
      The device measures CO₂ levels, shows them locally on the LCD, and publishes JSON data to an MQTT broker (HiveMQ Cloud) for remote monitoring via a custom app or dashboard.

## ✨ Features

      📟 Real-time CO₂ monitoring with two MH-Z19 sensors
      🖥️ LCD display for local readout
      ☁️ MQTT publish (over TLS) to HiveMQ Cloud
      📱 Easy integration with mobile or web dashboards
      📊 JSON payload for interoperability with any IoT platform

## 🛠️ Hardware Used

      ESP32 DevKit V1
      2x MH-Z19 (or compatible) CO₂ sensors (UART)
      1x I²C LCD (1602/1604)
      Breadboard + jumper wires

## 📐 Wiring Overview

      CO₂ Sensor 1 → ESP32 UART1 (RX=16, TX=17)
      CO₂ Sensor 2 → ESP32 UART2 (RX=4, TX=2)
      LCD → I²C (SDA=GPIO21, SCL=GPIO22 by default)
      Power: 5V + GND shared across ESP32 and sensors

## 💻 Installation & Setup

      Install Arduino IDE 
      Add ESP32 board support via Board Manager

Install libraries:
      - WiFi.h
      - WiFiClientSecure.h
      - PubSubClient.h
      - ArduinoJson.h
      - LiquidCrystal_I2C.h

Clone this repo:

      git clone https://github.com/K-IMAD/esp32-co2-monitor.git
      cd esp32-co2-monitor

Open .ino in Arduino IDE, update your:

      Wi-Fi SSID & password
      MQTT broker, port, username, password
      Device ID
      Upload to your ESP32

## 📊 Example MQTT JSON Payload

When publishing, the ESP32 sends:

      {
        "device": "esp32-co2-01",
        "co2_1": 950,
        "co2_2": 980,
        "ts": 1736809212
      }


Topic:
sensors/esp32-co2-01/co2

## 📸 System Overview

      CO₂ Sensors --> ESP32 --> Wi-Fi --> MQTT Broker (HiveMQ) --> App/Dashboard

## 🚀 Future Enhancements

* Add buzzer or relay control when CO₂ exceeds safe limits
* Store historical data (SD card or SPIFFS)
* Build a real-time web dashboard hosted on ESP32
* Add support for more environmental sensors (Temp/Humidity/TVOC)

## 📄 License

This project is open-source under the MIT License.

## Project diagram:
<img width="1536" height="1024" alt="project_recap" src="https://github.com/user-attachments/assets/e381ee2c-226f-4c7f-b6d0-e2d0b15bc669" />


