# Deployment Guide

## Hardware Deployment Options

### Option 1: Mains-Powered (Recommended for Development)
**Use Case**: Indoor monitoring, continuous real-time dashboard

**Connections:**
- ESP32 powered via USB cable (5V → VIN)
- LCD powered from ESP32 5V rail
- DHT22 powered from ESP32 3.3V rail

**Configuration:**
```cpp
#define ENABLE_DEEP_SLEEP  false
#define READ_INTERVAL_MS   2000
```

**Expected Performance:**
- Update rate: Every 2 seconds
- WiFi: Always connected
- LCD: Always on
- Power draw: ~240mA during TX, ~80mA idle

### Option 2: Battery-Powered (Field Deployment)
**Use Case**: Remote monitoring, long-term unattended operation

**Components Added:**
- 18650 Li-Ion Battery (2000mAh)
- TP4056 Charging Module with protection
- FireBeetle ESP32 (low quiescent current)

**Connections:**
- Battery → TP4056 → ESP32 VIN
- DHT22 VCC controlled via GPIO (MOSFET switch)
- LCD backlight disabled or on GPIO control

**Configuration:**
```cpp
#define ENABLE_DEEP_SLEEP  true
#define SLEEP_DURATION_MIN 10
#define READ_INTERVAL_MS   2000
```

**Expected Performance:**
- Update rate: Every 10 minutes
- Sleep current: ~10µA
- Active current: ~240mA for 2 seconds
- Battery life: ~260 days on 2000mAh

**Power Budget Calculation:**
```
Active cycle (2s):  240mA × 2s = 480mAs = 0.133mAh
Sleep cycle (598s): 10µA × 598s = 5.98mAs = 0.00166mAh
Total per 10min:    0.133 + 0.00166 = 0.135mAh
Daily consumption:  0.135mAh × 144 = 19.44mAh
Battery life:       2000mAh / 19.44mAh/day = 102 days

With FireBeetle (0.011mA deep sleep):
Sleep cycle:        0.011mA × 598s = 6.58mAs = 0.00183mAh
Total per 10min:    0.133 + 0.00183 = 0.135mAh
Daily:              19.44mAh
Battery life:       2000 / 19.44 = 102 days (similar, but more stable)
```

### Option 3: Solar-Powered (Sustainable Deployment)
**Use Case**: Outdoor weather station, permanent installation

**Components Added:**
- 5V Solar Panel (5W)
- 18650 Battery + TP4056
- Weatherproof enclosure (IP65)
- Waterproof DHT22 probe

**Configuration:**
```cpp
#define ENABLE_DEEP_SLEEP  true
#define SLEEP_DURATION_MIN 15  // Less frequent updates to save power
```

## Software Deployment Steps

### Step 1: Install Arduino IDE
1. Download from [arduino.cc](https://www.arduino.cc/en/software)
2. Install ESP32 board support:
   - File → Preferences → Additional Board Manager URLs:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
   - Tools → Board → Boards Manager → Search "ESP32" → Install

### Step 2: Install Libraries
1. Sketch → Include Library → Manage Libraries
2. Install each library:
   - `DHT sensor library` by Adafruit (≥1.4.4)
   - `Adafruit Unified Sensor` by Adafruit (≥1.1.9)
   - `Blynk` by Volodymyr Shymanskyy (≥1.3.2)
   - `LiquidCrystal_I2C` by Frank de Brabander (≥1.1.4)

### Step 3: Configure Credentials
1. Copy `config_template.h` to `config.h`
2. Edit `config.h` with your credentials:
   ```cpp
   #define BLYNK_AUTH_TOKEN "YourActualTokenHere"
   #define WIFI_SSID "YourWiFiName"
   #define WIFI_PASSWORD "YourWiFiPassword"
   ```
3. **IMPORTANT**: Add `config.h` to `.gitignore` to protect credentials

### Step 4: Upload Firmware
1. Connect ESP32 via USB
2. Select board: Tools → Board → ESP32 Arduino → ESP32 Dev Module
3. Select port: Tools → Port → COMx (Windows) or /dev/ttyUSBx (Linux)
4. Click Upload button (→)
5. Wait for "Done uploading" message

### Step 5: Verify Operation
1. Open Serial Monitor (Tools → Serial Monitor)
2. Set baud rate to 115200
3. Expected output:
   ```
   [2s] [INFO]  === ESP32 DHT22 IoT Monitor v2.0 ===
   [2s] [INFO]  Initializing subsystems...
   [3s] [INFO]  DHT22 sensor initialized on GPIO4
   [5s] [INFO]  WiFi connected
   [5s] [INFO]  IP Address: 192.168.1.105
   [6s] [INFO]  Blynk connected successfully
   [8s] [INFO]  timestamp,uptime_s,temperature_c,humidity_pct,heat_index_c,dew_point_c,wifi_rssi_dbm,status
   [10s] [DEBUG] Blynk data sent successfully
   [10s] [INFO]  10,10,28.50,65.20,31.20,21.40,-52,OK
   ```

### Step 6: Configure Blynk Dashboard
1. Open Blynk app or web console
2. Create new device from template
3. Add widgets:
   - Gauge: V0 (Temperature) — Red gradient, 0-50°C
   - Gauge: V1 (Humidity) — Blue gradient, 0-100%
   - Gauge: V2 (Heat Index) — Orange gradient, 0-50°C
   - Chart: V0, V1, V2 — Time range: 1 day
   - Label: V4 (WiFi RSSI)
   - Label: V5 (Uptime)
4. Add Event: `high_temp_alert` → Push notification
5. Save dashboard

## Testing & Validation

### Sensor Accuracy Test
Upload `tests/sensor_validation.ino`:
```cpp
// Compare DHT22 against calibrated reference thermometer
// Run for 24 hours, log every minute
// Expected: ±0.5°C accuracy, ±2% RH accuracy
```

### WiFi Stress Test
Upload `tests/wifi_stress_test.ino`:
```cpp
// Continuously connect/disconnect WiFi
// Monitor reconnection time and success rate
// Expected: <5s reconnect, >99% success rate
```

### Power Profile Test
Upload `tests/power_profile.ino`:
```cpp
// Measure current in different modes
// Use multimeter in series with power supply
// Expected: Active 240mA, Sleep 10µA
```

## Troubleshooting Deployment Issues

| Issue | Diagnostic | Solution |
|-------|------------|----------|
| Upload fails | Wrong COM port / driver issue | Install CH340/CP210x driver; try different USB cable |
| Blynk offline | Auth token mismatch | Regenerate token in Blynk console; update config.h |
| LCD blank | Wrong I2C address | Scan I2C: `Wire.begin(); Wire.scan()`; try 0x3F |
| DHT22 nan | Wiring / power issue | Check 10kΩ pull-up; add 100µF cap; verify 3.3V stability |
| WiFi weak | Distance / interference | Move closer to router; use 2.4GHz band; check antenna |
| High temp readings | ESP32 heat transfer | Separate DHT22 by 10cm+; use longer wires |
| Battery drains fast | Sleep not working | Verify `ENABLE_DEEP_SLEEP`; check wake source config |

## Maintenance Schedule

| Task | Frequency | Action |
|------|-----------|--------|
| Sensor calibration | Monthly | Compare with reference; apply offset if needed |
| WiFi credential rotation | Quarterly | Update password; reconfigure device |
| Battery check | Monthly | Measure voltage; replace if <3.0V |
| Firmware update | As needed | Upload new version via OTA or USB |
| Data backup | Weekly | Export CSV from serial logs or Blynk history |
