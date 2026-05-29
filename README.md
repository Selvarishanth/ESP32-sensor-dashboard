# 🌡️ IoT-Based Temperature & Humidity Monitoring Dashboard

![Code](c:\Users\Hacke\Downloads\esp32_dht22_blynk.ino)
![Circuit Diagram](c:\Users\Hacke\AppData\Local\GitHubDesktop\app-3.5.11\Public\ESP32 sensor dashboard\Circuit Diagram.png)
![IoT](https://blynk.cloud)
  
> A real-time IoT system that reads temperature, humidity, and heat index using a DHT22 sensor on an ESP32, displays live data on a 16x2 LCD, and streams it to a cloud dashboard via Blynk — accessible from anywhere on your phone or browser.

---

## 📸 Project Overview

| Component | Details |
|-----------|---------|
| Microcontroller | ESP32 Dev Module |
| Sensor | DHT22 (Temperature + Humidity) |
| Display | 16x2 I2C LCD (0x27 / 0x3F) |
| Cloud Platform | Blynk IoT (free tier) |
| Communication | WiFi (802.11 b/g/n) |
| Update Rate | Every 2 seconds |

---

## 🔧 Hardware Used

- ESP32 Development Board
- DHT22 Temperature & Humidity Sensor
- 16x2 I2C LCD Display
- 10kΩ Pull-up Resistor
- Breadboard + Jumper Wires
- USB Cable (for power/programming)

---

## 📐 Circuit Diagram

> See `images/circuit_diagram.png` in this repository.

### Wiring Summary

| DHT22 Pin | ESP32 Pin |
|-----------|-----------|
| VCC (Pin 1) | 3.3V |
| DATA (Pin 2) | GPIO 4 |
| GND (Pin 4) | GND |
| — | 10kΩ resistor between DATA and 3.3V |

| LCD (I2C) Pin | ESP32 Pin |
|---------------|-----------|
| VCC | 5V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

---

## 💡 Features

- 📡 **Live WiFi Streaming** — Data sent to Blynk cloud every 2 seconds
- 🌡️ **Temperature Reading** — Accurate to ±0.5°C (DHT22)
- 💧 **Humidity Reading** — Accurate to ±2–5% RH
- 🔥 **Heat Index Calculation** — "Feels like" temperature computed onboard
- 📺 **Local LCD Display** — Shows T, H, and WiFi status without needing a phone
- 📊 **Cloud Dashboard** — Gauge widgets on Blynk web/app with historical graphs
- 🔁 **Auto-Reconnect** — Handles WiFi drops gracefully

---

## 📲 Blynk Dashboard Setup

1. Create a free account at [blynk.cloud](https://blynk.cloud)
2. Create a **New Template** → name it `DHT22 Dashboard`
3. Add **Datastreams**:

| Virtual Pin | Name | Type | Range |
|-------------|------|------|-------|
| V0 | Temperature | Double | -40 to 80 |
| V1 | Humidity | Double | 0 to 100 |
| V2 | Heat Index | Double | -40 to 80 |

4. Add **Gauge widgets** on the Web Dashboard, assign V0, V1, V2
5. Copy your **Auth Token** from the device settings

---

## 🖥️ What the LCD Shows

```
Line 1:  T:28.5°C   WiFi:+++
Line 2:  H:65.2%
```

- `T:` → live temperature
- `H:` → live humidity
- `WiFi:+++` → connected | `WiFi:---` → reconnecting

---

## 📊 Sample Readings

| Condition | Temperature | Humidity | Heat Index |
|-----------|-------------|----------|------------|
| Room (AC on) | 23.4°C | 52% | 22.8°C |
| Room (no AC) | 31.2°C | 74% | 34.6°C |
| Outdoor shade | 36.1°C | 68% | 41.2°C |

---

## 🚀 Getting Started

### 1. Install Libraries (Arduino IDE)

Go to **Tools → Manage Libraries** and install:

| Library | Author |
|---------|--------|
| DHT sensor library | Adafruit |
| Adafruit Unified Sensor | Adafruit |
| Blynk | Volodymyr Shymanskyy |
| LiquidCrystal_I2C | Frank de Brabander |

### 2. Configure Credentials

Open `code/esp32_dht22_blynk.ino` and update:

```cpp
#define BLYNK_TEMPLATE_ID   "Your_Template_ID"
#define BLYNK_AUTH_TOKEN    "Your_Auth_Token"

char ssid[] = "Your_WiFi_Name";
char pass[] = "Your_WiFi_Password";
```

### 3. Upload

- Board: **ESP32 Dev Module**
- Baud: **115200**
- Select correct COM port → click **Upload**

### 4. Monitor

- Open **Serial Monitor** at 115200 baud to see CSV output
- Open **Blynk app/web** to see live dashboard
- Check your LCD for local readings

---

## 📁 Repository Structure

```
iot-dht22-dashboard/
├── code/
│   └── esp32_dht22_blynk.ino     ← Main Arduino code
├── images/
│   ├── circuit_diagram.png        ← Block/circuit diagram
│   └── project_photo.jpg          ← Physical setup photo
└── README.md
```

---

## 🔮 Future Improvements

- [ ] Add OLED display for richer UI
- [ ] Log data to Google Sheets via Google Apps Script
- [ ] Add threshold-based alerts (SMS/email via Blynk Automation)
- [ ] Battery-powered version with deep sleep for low power
- [ ] Multiple sensor nodes with MQTT broker (scalable deployment)
- [ ] Export CSV data for analysis in Python/Excel

---

## 🛠️ Troubleshooting

| Issue | Fix |
|-------|-----|
| LCD shows blank / black boxes | Change `0x27` to `0x3F` on the `LiquidCrystal_I2C` init line |
| Blynk won't connect | Check Auth Token and WiFi credentials |
| DHT22 shows `nan` values | Check 10kΩ pull-up resistor on DATA pin |
| Serial garbage output | Set baud rate to 115200 in Serial Monitor |

---

## 👤 Author

**[Selvarishanth]**
B.E. / B.Tech — [ Electronics And Instrumentation Engineering], [SRMIST]
📧 [Selvarishatnh.sridhar@example.com]
🔗 [LinkedIn Profile](https://www.linkedin.com/in/selvarishanth-sridhar?utm_source=share&utm_campaign=share_via&utm_content=profile&utm_medium=android_app)

---

## 📄 License

This project is licensed under the MIT License — feel free to use, modify, and build on it.

---

> ⭐ If you found this useful, consider starring the repo!
