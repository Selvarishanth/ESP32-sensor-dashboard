# System Architecture

## Overview

This document describes the system architecture of the IoT Temperature & Humidity Monitoring System, covering hardware, firmware, cloud integration, and data flow.

## System Components

### 1. Hardware Layer

```
┌─────────────────────────────────────────────────────────────┐
│                    HARDWARE LAYER                            │
│                                                              │
│  ┌─────────────┐      ┌─────────────┐      ┌─────────────┐ │
│  │   DHT22     │      │    ESP32    │      │  I2C LCD    │ │
│  │  AM2302     │──────│  WROOM-32   │──────│   16×2      │ │
│  │  (GPIO4)    │      │  240MHz     │      │  (I2C)      │ │
│  └─────────────┘      │  WiFi/BT    │      └─────────────┘ │
│                       │  520KB SRAM │                        │
│                       └─────────────┘                        │
│                              │                               │
│                              ▼                               │
│                       ┌─────────────┐                        │
│                       │  10kΩ Pull  │                        │
│                       │  Up Resistor│                        │
│                       └─────────────┘                        │
└─────────────────────────────────────────────────────────────┘
```

**ESP32 Pin Allocation:**
| Pin | Function | Direction | Notes |
|-----|----------|-----------|-------|
| GPIO4 | DHT22 DATA | Input | 10kΩ pull-up to 3.3V |
| GPIO21 | I2C SDA | Bidirectional | LCD data line |
| GPIO22 | I2C SCL | Output | LCD clock line |
| 3.3V | DHT22 VCC | Power | Stable supply required |
| 5V (VIN) | LCD VCC | Power | USB or external 5V |
| GND | Common Ground | Power | Single point ground |

### 2. Firmware Layer

```
┌─────────────────────────────────────────────────────────────┐
│                   FIRMWARE LAYER                             │
│                                                              │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐        │
│  │  Sensor     │  │   Math      │  │   Display   │        │
│  │  Driver     │  │  Engine     │  │   Manager   │        │
│  │  (DHT.h)    │  │(Heat Index, │  │(LCD+I2C)    │        │
│  │             │  │ Dew Point)  │  │             │        │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘        │
│         │                │                │                │
│         └────────────────┼────────────────┘                │
│                          │                                  │
│                   ┌──────┴──────┐                           │
│                   │   Main      │                           │
│                   │  Controller │                           │
│                   │  (loop())   │                           │
│                   └──────┬──────┘                           │
│                          │                                  │
│         ┌────────────────┼────────────────┐                │
│         │                │                │                │
│  ┌──────┴──────┐  ┌──────┴──────┐  ┌──────┴──────┐        │
│  │   WiFi      │  │   Blynk     │  │   Power     │        │
│  │  Manager    │  │   Client    │  │   Manager   │        │
│  │             │  │             │  │(Sleep modes)│        │
│  └─────────────┘  └─────────────┘  └─────────────┘        │
└─────────────────────────────────────────────────────────────┘
```

**Task Scheduling:**
- **Priority 1**: Blynk.run() — handles cloud communication (non-blocking)
- **Priority 2**: Sensor read — 2-second timer callback
- **Priority 3**: LCD update — triggered after successful sensor read
- **Priority 4**: Serial logging — CSV output for data analysis

### 3. Cloud Integration Layer

```
┌─────────────────────────────────────────────────────────────┐
│                   CLOUD LAYER                                │
│                                                              │
│  ┌─────────────┐      ┌─────────────┐      ┌─────────────┐ │
│  │   Blynk     │      │  Virtual    │      │   Event     │ │
│  │   Cloud     │──────│   Pins      │──────│   Engine    │ │
│  │  (blynk.io) │      │  (V0–V5)    │      │             │ │
│  └─────────────┘      └─────────────┘      └─────────────┘ │
│         │                      │                      │      │
│         │                      │                      │      │
│         ▼                      ▼                      ▼      │
│  ┌─────────────┐      ┌─────────────┐      ┌─────────────┐ │
│  │  Mobile App │      │  Web Dash   │      │  Push Notif │ │
│  │  (iOS/And)  │      │  (Browser)  │      │  (Email)    │ │
│  └─────────────┘      └─────────────┘      └─────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

**Datastream Mapping:**
| Virtual Pin | Data | Type | Update Rate | Widget |
|-------------|------|------|-------------|--------|
| V0 | Temperature | Double | 2s | Gauge |
| V1 | Humidity | Double | 2s | Gauge |
| V2 | Heat Index | Double | 2s | Gauge |
| V3 | Dew Point | Double | 2s | Label |
| V4 | WiFi RSSI | Integer | 2s | Label |
| V5 | Uptime | Integer | 2s | Label |

### 4. Data Flow Diagram

```
┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐
│  DHT22  │───→│  ESP32  │───→│  Blynk  │───→│  Mobile │
│ Sensor  │    │ Compute │    │  Cloud  │    │   App   │
└─────────┘    └────┬────┘    └─────────┘    └─────────┘
                    │
                    ├────→┌─────────┐
                    │     │  I2C    │
                    │     │  LCD    │
                    │     └─────────┘
                    │
                    └────→┌─────────┐
                          │ Serial  │
                          │  CSV    │
                          └─────────┘
```

**Processing Pipeline:**
1. **Acquisition** (2s): DHT22 reads T/H → ESP32 validates with `isnan()`
2. **Computation** (<1ms): Heat index & dew point calculated onboard
3. **Transmission** (200ms): Values sent to Blynk via WiFi TCP
4. **Display** (50ms): LCD updated with formatted strings
5. **Logging** (10ms): CSV line written to Serial

## Power States

```
┌─────────────┐      ┌─────────────┐      ┌─────────────┐
│   ACTIVE    │─────→│ MODEM SLEEP │─────→│  DEEP SLEEP │
│  ~240mA     │      │  ~80mA      │      │  ~10µA      │
│  WiFi TX    │      │  CPU Active │      │  RTC Timer  │
│  Sensor On  │      │  WiFi Off   │      │  All Off    │
└─────────────┘      └─────────────┘      └─────────────┘
     ↑                                          │
     └──────────────────────────────────────────┘
              Timer Wake-up (10min)
```

## Error Handling Strategy

| Error Type | Detection | Response | Recovery |
|------------|-----------|----------|----------|
| Sensor Read Fail | `isnan()` check | LCD error msg, skip cycle | Auto-retry next interval |
| WiFi Disconnect | `WiFi.status()` | Attempt reconnect | Exponential backoff |
| Blynk Offline | `Blynk.connected()` | Local mode only | Auto-reconnect on next loop |
| Power Brownout | Watchdog timer | System reset | Preserve RTC memory |

## Scalability Path

```
Current:        1 ESP32 + 1 DHT22 + Blynk
                │
Phase 1:        1 ESP32 + 2 DHT22 (indoor/outdoor) + Blynk
                │
Phase 2:        3 ESP32 nodes + MQTT broker + Central dashboard
                │
Phase 3:        ESP32 + LoRaWAN → Gateway → Cloud → Analytics
                │
Phase 4:        ML prediction + Automated climate control
```
