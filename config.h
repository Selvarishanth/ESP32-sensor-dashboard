/*
 * config.h — User Configuration File
 * 
 * IMPORTANT: This file contains sensitive credentials.
 * For security, add this file to .gitignore and never commit it to public repos.
 * Use config_template.h as a reference for setup.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ═══════════════════════════════════════════════════════════════════════════
//  BLYNK IOT 2.0 CREDENTIALS
//  Get these from: https://blynk.cloud → Your Device → Device Info
// ═══════════════════════════════════════════════════════════════════════════

#define BLYNK_TEMPLATE_ID    "YOUR_TEMPLATE_ID_HERE"      // e.g., "TMPxxxxxxxxxx"
#define BLYNK_TEMPLATE_NAME  "DHT22 Environmental Monitor"
#define BLYNK_AUTH_TOKEN     "YOUR_AUTH_TOKEN_HERE"       // 32-character token

// ═══════════════════════════════════════════════════════════════════════════
//  WIFI CREDENTIALS
// ═══════════════════════════════════════════════════════════════════════════

#define WIFI_SSID     "Your_WiFi_Network_Name"
#define WIFI_PASSWORD "Your_WiFi_Password"

// ═══════════════════════════════════════════════════════════════════════════
//  HARDWARE CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

// DHT22 Sensor
#define DHT_PIN         4           // GPIO4 — DATA pin
#define DHT_TYPE        DHT22       // AM2302 (also compatible with DHT11, DHT21)

// I2C LCD Display (16x2 with PCF8574T backpack)
#define LCD_I2C_ADDR    0x27        // Common: 0x27 or 0x3F
#define LCD_COLS        16
#define LCD_ROWS        2

// ═══════════════════════════════════════════════════════════════════════════
//  OPERATIONAL PARAMETERS
// ═══════════════════════════════════════════════════════════════════════════

// Sensor read interval (milliseconds)
// Minimum for DHT22: 2000ms (sensor requires 2s between reads)
#define READ_INTERVAL_MS    2000

// Alert thresholds
#define TEMP_ALERT_HIGH_C   35.0    // High temperature alert (°C)
#define TEMP_ALERT_LOW_C    10.0    // Low temperature warning (°C)
#define HUMIDITY_ALERT_HIGH 85.0    // High humidity / mold risk (%)

// ═══════════════════════════════════════════════════════════════════════════
//  POWER MANAGEMENT
// ═══════════════════════════════════════════════════════════════════════════

// Enable deep sleep for battery-powered operation
// When enabled, device sleeps between readings to save power
#define ENABLE_DEEP_SLEEP   false

// Sleep duration in minutes (only used when deep sleep is enabled)
// Recommended: 5-15 minutes for battery, 1-2 seconds for mains power
#define SLEEP_DURATION_MIN  10

// ═══════════════════════════════════════════════════════════════════════════
//  DEBUG & LOGGING
// ═══════════════════════════════════════════════════════════════════════════

// Debug output level: 0=Off, 1=Errors only, 2=Info, 3=Verbose
#define DEBUG_LEVEL         2

// Enable serial CSV output for data logging
#define ENABLE_CSV_LOG      true

// ═══════════════════════════════════════════════════════════════════════════
//  BLYNK VIRTUAL PIN ASSIGNMENTS
// ═══════════════════════════════════════════════════════════════════════════

#define VPIN_TEMPERATURE    V0
#define VPIN_HUMIDITY       V1
#define VPIN_HEAT_INDEX     V2
#define VPIN_DEW_POINT      V3
#define VPIN_WIFI_RSSI      V4
#define VPIN_UPTIME         V5
#define VPIN_SYSTEM_STATUS  V6

#endif // CONFIG_H
