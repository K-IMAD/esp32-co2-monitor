🌱 ESP32 CO₂ Dual-Sensor Monitor 
      This project uses an ESP32 microcontroller with two CO₂ sensors to monitor air quality in real time.
      Results are displayed on an I²C LCD (1602/1604) and indicated using LEDs (traffic-light style).

✨ Features
      📟 Real-time CO₂ monitoring with two sensors
      🖥️ LCD display of both sensor values
      ⚡ Built with Arduino framework on ESP32

🛠️ Hardware Used
    ESP32 DevKit V1
    2x CO₂ Sensors (UART-based, e.g., MH-Z19 or MH-Z19B)
    1x I²C LCD 1602/1604 module
    Breadboard + jumper wires

📐 Wiring Overview
    CO₂ Sensor 1 → ESP32 UART1 (GPIO16 RX, GPIO17 TX)
    CO₂ Sensor 2 → ESP32 UART2 (GPIO4 RX, GPIO2 TX)
    LCD → I²C (SDA=GPIO21, SCL=GPIO22 by default)
