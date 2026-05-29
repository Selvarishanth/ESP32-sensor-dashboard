# Troubleshooting Guide

## Quick Diagnostic Flowchart

```
Device Won't Power On?
    │
    ├─→ Check USB cable (try another)
    ├─→ Check power LED on ESP32
    ├─→ Try different USB port / power supply
    └─→ Measure voltage at 3.3V pin (should be 3.25–3.35V)

Serial Monitor Shows Nothing?
    │
    ├─→ Set baud rate to 115200
    ├─→ Select correct COM port
    ├─→ Press EN (reset) button
    └─→ Check USB driver (CH340/CP210x)

DHT22 Returns nan?
    │
    ├─→ Verify 10kΩ pull-up resistor between DATA and 3.3V
    ├─→ Check wiring: VCC→3.3V, DATA→GPIO4, GND→GND
    ├─→ Add 100µF capacitor across 3.3V/GND
    ├─→ Increase read interval to 2000ms minimum
    ├─→ Try different GPIO pin (avoid GPIO0, GPIO2, GPIO12, GPIO15)
    └─→ Replace DHT22 (sensor may be faulty)

LCD Shows Black Boxes?
    │
    ├─→ Check I2C address: try 0x27 or 0x3F
    ├─→ Verify SDA→GPIO21, SCL→GPIO22
    ├─→ Check LCD contrast potentiometer (on I2C backpack)
    ├─→ Test with I2C scanner sketch
    └─→ Check LCD backlight jumper (should be connected)

Blynk Not Connecting?
    │
    ├─→ Verify BLYNK_AUTH_TOKEN in config.h
    ├─→ Check WiFi credentials (case-sensitive!)
    ├─→ Ensure device is added to Blynk template
    ├─→ Check firewall (port 80/443 open)
    ├─→ Verify Blynk library version ≥1.3.2
    └─→ Try Blynk.config() + Blynk.connect() instead of Blynk.begin()

WiFi Keeps Disconnecting?
    │
    ├─→ Check RSSI: should be > -70 dBm
    ├─→ Move closer to router
    ├─→ Use 2.4GHz band (not 5GHz)
    ├─→ Add WiFi.setAutoReconnect(true)
    ├─→ Check router DHCP lease time
    └─→ Reduce WiFi power: WiFi.setTxPower(WIFI_POWER_11dBm)

Temperature Readings Too High?
    │
    ├─→ ESP32 heat transfer: separate sensor by 10cm+
    ├─→ Check for direct sunlight on sensor
    ├─→ Verify sensor not near heat sources
    ├─→ Allow 5-minute stabilization after power-on
    └─→ Apply calibration offset if consistent bias

Battery Drains Too Fast?
    │
    ├─→ Verify ENABLE_DEEP_SLEEP is true
    ├─→ Check sleep current with multimeter (should be ~10µA)
    ├─→ Ensure DHT22 powered off during sleep (use GPIO + MOSFET)
    ├─→ Disable serial debug: #define DEBUG_LEVEL 0
    ├─→ Use FireBeetle ESP32 (low quiescent current regulator)
    └─→ Reduce update frequency: SLEEP_DURATION_MIN 15+
```

## Detailed Error Messages

### Error: "DHT22 read failed — check wiring/pull-up resistor"
**Cause**: Sensor not responding to data request
**Solutions**:
1. Verify 10kΩ resistor between DHT22 DATA and 3.3V
2. Check all solder joints / breadboard connections
3. Ensure DHT22 pinout is correct (VCC=Pin1, DATA=Pin2, NC=Pin3, GND=Pin4)
4. Test with simple DHT test sketch (without Blynk/LCD)
5. Replace sensor if consistently failing (>5% error rate)

### Error: "WiFi connection failed! Restarting..."
**Cause**: Cannot connect to specified network
**Solutions**:
1. Double-check SSID and password (case-sensitive, no trailing spaces)
2. Ensure 2.4GHz network (ESP32 does not support 5GHz)
3. Check if network uses WPA2 (not WPA3 or enterprise)
4. Verify router is not in MAC address whitelist mode
5. Try connecting to mobile hotspot to isolate router issue

### Error: "Blynk connection failed! Running offline mode."
**Cause**: Cannot reach Blynk cloud servers
**Solutions**:
1. Verify BLYNK_AUTH_TOKEN is correct (32 characters)
2. Check if device is activated in Blynk console (not just created)
3. Ensure template ID matches device template
4. Test internet connectivity: ping blynk.cloud from another device
5. Check if Blynk servers are operational: status.blynk.io

### Warning: "Temperature is 36.2°C (threshold: 35°C)"
**Cause**: Ambient temperature exceeded alert threshold
**Actions**:
- This is normal behavior if environment is actually hot
- Adjust threshold in config.h if needed
- Check if sensor is affected by ESP32 heat (move further away)
- Verify sensor accuracy with reference thermometer

## Performance Optimization

### Reduce Power Consumption
```cpp
// In config.h:
#define ENABLE_DEEP_SLEEP   true
#define SLEEP_DURATION_MIN  15
#define DEBUG_LEVEL         0

// In setup(), after Blynk connection:
WiFi.setTxPower(WIFI_POWER_11dBm);  // Reduce WiFi power
setCpuFrequencyMhz(80);               // Minimum for WiFi
```

### Improve WiFi Stability
```cpp
// Add to setup():
WiFi.setAutoReconnect(true);
WiFi.persistent(true);
WiFi.setSleep(false);  // Disable WiFi sleep for stable connection
```

### Optimize Sensor Readings
```cpp
// Add filtering to reduce noise:
float tempFiltered = 0.9 * lastTemp + 0.1 * newTemp;  // Low-pass filter
```

## Serial Debug Commands

While connected to Serial Monitor, send these commands:

| Command | Action | Response |
|---------|--------|----------|
| `r` | Force sensor read | Current T/H values |
| `i` | Show system info | IP, RSSI, uptime, version |
| `s` | Enter deep sleep | "Sleeping..." message |
| `w` | WiFi reconnect | Connection status |
| `c` | Clear LCD | LCD blank + reinitialize |
| `d` | Toggle debug level | Current level + change |

## Contact & Support

If issues persist after following this guide:
1. Check [GitHub Issues](https://github.com/selvarishanth/iot-dht22-dashboard/issues)
2. Include serial log output (first 30 seconds after boot)
3. Specify: ESP32 board model, DHT22 model, Arduino IDE version, Blynk version
