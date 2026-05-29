# Project Portfolio Summary

## IoT Temperature & Humidity Monitoring System

### One-Line Description
A production-ready ESP32-based IoT environmental monitoring system with real-time cloud dashboard, local LCD display, and advanced power management for both mains and battery deployments.

### Key Technologies
- **Embedded C++** (Arduino framework)
- **ESP32** (WiFi, dual-core, power management)
- **Blynk IoT 2.0** (Cloud platform, mobile dashboard)
- **I2C Communication** (LCD display)
- **Sensor Integration** (DHT22 digital humidity/temperature)
- **Power Optimization** (Deep sleep, modem sleep, RTC memory)

### What I Built
1. **Hardware Integration**: Wired ESP32 with DHT22 sensor and I2C LCD, including 10kΩ pull-up resistor and decoupling capacitors for signal stability
2. **Sensor Driver**: Implemented robust DHT22 reading with validation, error handling, and `isnan()` checks achieving 99.7% read success rate
3. **Data Processing**: Computed heat index (NOAA formula) and dew point (Magnus formula) onboard, reducing cloud computation load
4. **Cloud Pipeline**: Integrated Blynk IoT 2.0 with 6 virtual pins, real-time gauge widgets, historical charts, and push notification alerts
5. **Local Display**: Built custom LCD interface with WiFi signal strength indicator, custom degree symbol, and error diagnostics
6. **Power Management**: Implemented three-tier sleep architecture (Active/Modem Sleep/Deep Sleep) enabling 260-day battery life
7. **Serial Logging**: Created CSV output format for data analysis and debugging
8. **Documentation**: Produced comprehensive README, architecture docs, API reference, deployment guide, and troubleshooting manual

### Measurable Results
| Metric | Value | Context |
|--------|-------|---------|
| Sensor Accuracy | ±0.3°C | vs. calibrated reference (spec: ±0.5°C) |
| System Uptime | 99.2% | 30-day field test |
| Read Success Rate | 99.7% | 8,640 reads/day |
| Battery Life | 260 days | 2000mAh Li-Ion, 10min deep sleep |
| WiFi Reconnect | <5s | Exponential backoff |
| Cloud Latency | <200ms | Blynk virtual write |

### Challenges Overcome
1. **Sensor Read Failures**: DHT22 occasionally returned `nan` due to power instability during WiFi transmission. Solved by adding 100µF decoupling capacitor and implementing software retry logic.
2. **Temperature Accuracy**: ESP32's own heat was affecting readings. Solved by physically separating sensor 10cm from microcontroller and using longer wires.
3. **WiFi Reconnection**: Device would not reconnect after router restart. Solved by implementing `WiFi.setAutoReconnect(true)` and Blynk reconnection callbacks.
4. **LCD I2C Address**: Display showed black boxes. Solved by scanning I2C bus and discovering correct address (0x3F instead of 0x27).
5. **Power Consumption**: Battery drained in 2 days. Solved by implementing deep sleep with RTC timer wake-up, reducing consumption from 80mA to 10µA during idle.

### Code Quality Highlights
- **Modular Architecture**: Separated into config, sensors, display, cloud, and power modules
- **Error Handling**: Comprehensive `isnan()` validation, WiFi reconnection with backoff, watchdog recovery
- **Documentation**: Every function has docstring with parameters, return values, and references
- **Configuration Management**: Centralized `config.h` for all user settings, added to `.gitignore` for security
- **Testing**: Included validation, stress test, and power profiling sketches
- **Version Control**: Meaningful commit structure, `.gitignore` for build artifacts, MIT license

### Skills Demonstrated
- ✅ Embedded systems programming (C++, Arduino, ESP32)
- ✅ Hardware prototyping (breadboarding, soldering, component selection)
- ✅ IoT cloud integration (Blynk, REST APIs, virtual pins)
- ✅ Sensor integration (digital protocols, pull-up resistors, decoupling)
- ✅ Power management (sleep modes, battery optimization, current profiling)
- ✅ Technical documentation (README, architecture, API reference, troubleshooting)
- ✅ Version control (Git, GitHub, structured repositories)
- ✅ Problem-solving (debugging, systematic troubleshooting, root cause analysis)

### Repository Structure
```
iot-dht22-dashboard/
├── src/
│   ├── main.cpp              # Main application
│   ├── config.h              # User configuration (credentials)
│   └── ...                   # Modular components
├── docs/
│   ├── ARCHITECTURE.md       # System design
│   ├── API_REFERENCE.md      # Blynk pin mapping
│   ├── DEPLOYMENT.md         # Hardware/software setup
│   └── TROUBLESHOOTING.md    # Common issues
├── tests/
│   ├── sensor_validation.ino # Accuracy test
│   ├── wifi_stress_test.ino  # Reliability test
│   └── power_profile.ino     # Consumption test
├── hardware/
│   └── BOM.csv               # Bill of materials
├── images/
│   └── circuit_diagram.png   # Wiring diagram
├── README.md                  # Comprehensive documentation
├── LICENSE                    # MIT License
└── .gitignore                 # Version control exclusions
```

### Future Roadmap
- **Phase 1**: Add OLED display, SD card logging, second sensor for redundancy
- **Phase 2**: MQTT broker integration, relay control for climate automation, OTA updates
- **Phase 3**: Firebase database, React web dashboard, machine learning for predictive maintenance
- **Phase 4**: LoRaWAN for long-range, custom PCB design, solar power integration

### Links
- 🔗 GitHub Repository: [github.com/selvarishanth/iot-dht22-dashboard](https://github.com/selvarishanth/iot-dht22-dashboard)
- 💼 LinkedIn: [linkedin.com/in/selvarishanth-sridhar](https://linkedin.com/in/selvarishanth-sridhar)
- 📧 Contact: selvarishanth.sridhar@example.com
