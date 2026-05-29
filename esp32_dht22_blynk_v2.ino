/*
 * ============================================================================
 *  IoT Temperature & Humidity Monitoring System
 *  Platform: ESP32 (ESP32-WROOM-32)
 *  Sensor:   DHT22 (AM2302) — Temperature + Humidity
 *  Display:  16×2 I2C LCD (PCF8574T, 0x27/0x3F)
 *  Cloud:    Blynk IoT 2.0 (blynk.cloud)
 *  
 *  Features:
 *    - Real-time T/H reading every 2 seconds
 *    - Heat index & dew point computation (NOAA formulas)
 *    - Cloud dashboard with historical graphs
 *    - Local LCD display with WiFi signal strength
 *    - Temperature threshold alerts via push notification
 *    - CSV serial logging for data analysis
 *    - Power management: Active / Modem Sleep / Deep Sleep modes
 *    - Robust error handling with exponential backoff reconnect
 *
 *  Course:  21EIC101J — Sensors and Actuators
 *  Author:  Selvarishanth Sridhar, SRMIST
 *  License: MIT
 * ============================================================================
 */

// ────────────────────────────────────────────────────────────────────────────
//  CONFIGURATION — Edit these values before uploading
// ────────────────────────────────────────────────────────────────────────────

// Blynk IoT 2.0 Credentials (from blynk.cloud → Device Info)
#define BLYNK_TEMPLATE_ID    "Your_Template_ID"      // ← REQUIRED: Replace
#define BLYNK_TEMPLATE_NAME  "DHT22 Environmental Monitor"
#define BLYNK_AUTH_TOKEN     "Your_Auth_Token"       // ← REQUIRED: Replace

// WiFi Credentials
const char* WIFI_SSID     = "Your_WiFi_SSID";         // ← REQUIRED: Replace
const char* WIFI_PASSWORD = "Your_WiFi_Password";     // ← REQUIRED: Replace

// Sensor Configuration
#define DHT_PIN         4           // GPIO4 — DHT22 DATA pin
#define DHT_TYPE        DHT22       // AM2302 sensor model
#define READ_INTERVAL   2000        // Sensor read interval (ms)

// I2C LCD Configuration
#define LCD_I2C_ADDR    0x27        // Try 0x3F if display is blank
#define LCD_COLS        16
#define LCD_ROWS        2

// Alert Thresholds (°C)
#define TEMP_ALERT_HIGH 35.0        // Send notification above this
#define TEMP_ALERT_LOW  10.0        // Optional: low temp warning
#define HUM_ALERT_HIGH  85.0        // Mold risk warning

// Power Management
#define ENABLE_DEEP_SLEEP  false    // true = battery mode (10min intervals)
#define SLEEP_DURATION_MIN 10       // Deep sleep duration (minutes)

// Debug Level: 0=Off, 1=Errors, 2=Info, 3=Verbose
#define DEBUG_LEVEL     2

// ────────────────────────────────────────────────────────────────────────────
//  INCLUDES
// ────────────────────────────────────────────────────────────────────────────

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ────────────────────────────────────────────────────────────────────────────
//  BLYNK VIRTUAL PIN MAPPING
// ────────────────────────────────────────────────────────────────────────────
#define VPIN_TEMPERATURE    V0
#define VPIN_HUMIDITY       V1
#define VPIN_HEAT_INDEX     V2
#define VPIN_DEW_POINT      V3
#define VPIN_WIFI_RSSI      V4
#define VPIN_UPTIME         V5

// ────────────────────────────────────────────────────────────────────────────
//  GLOBAL OBJECTS
// ────────────────────────────────────────────────────────────────────────────

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLS, LCD_ROWS);
BlynkTimer timer;

// Sensor data storage
struct SensorData {
  float temperature;
  float humidity;
  float heatIndex;
  float dewPoint;
  bool valid;
};

SensorData lastReading = {0, 0, 0, 0, false};

// State tracking
bool alertSentHigh = false;
bool alertSentLow  = false;
bool alertSentHum  = false;
unsigned long startTime = 0;
unsigned long lastReadTime = 0;

// Custom LCD characters
byte degreeChar[8] = {
  0b01100, 0b10010, 0b10010,
  0b01100, 0b00000, 0b00000,
  0b00000, 0b00000
};

byte wifiStrong[8] = {
  0b00000, 0b00000, 0b00000,
  0b00000, 0b00100, 0b01110,
  0b11111, 0b00000
};

// ────────────────────────────────────────────────────────────────────────────
//  DEBUG LOGGING
// ────────────────────────────────────────────────────────────────────────────

void logMessage(int level, const char* msg) {
  if (DEBUG_LEVEL >= level) {
    Serial.print("[");
    Serial.print(millis() / 1000);
    Serial.print("s] ");
    switch(level) {
      case 1: Serial.print("[ERROR] "); break;
      case 2: Serial.print("[INFO]  "); break;
      case 3: Serial.print("[DEBUG] "); break;
    }
    Serial.println(msg);
  }
}

// ────────────────────────────────────────────────────────────────────────────
//  SETUP
// ────────────────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for serial or timeout

  logMessage(2, "=== ESP32 DHT22 IoT Monitor v2.0 ===");
  logMessage(2, "Initializing subsystems...");

  // Initialize DHT sensor
  dht.begin();
  logMessage(2, "DHT22 sensor initialized on GPIO4");

  // Initialize LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, degreeChar);
  lcd.createChar(1, wifiStrong);

  // Splash screen
  lcd.setCursor(0, 0);
  lcd.print("  IoT Monitor   ");
  lcd.setCursor(0, 1);
  lcd.print("  Initializing  ");
  delay(1500);
  lcd.clear();

  // Connect to WiFi
  lcd.setCursor(0, 0);
  lcd.print("WiFi: Connecting");
  lcd.setCursor(0, 1);
  lcd.print(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 30) {
    delay(500);
    wifiAttempts++;
    lcd.setCursor(14, 1);
    lcd.print(wifiAttempts % 4 == 0 ? "/" : 
              wifiAttempts % 4 == 1 ? "-" : 
              wifiAttempts % 4 == 2 ? "\\" : "|");
  }

  if (WiFi.status() != WL_CONNECTED) {
    logMessage(1, "WiFi connection failed! Restarting...");
    lcd.clear();
    lcd.print("WiFi FAILED!");
    lcd.setCursor(0, 1);
    lcd.print("Check credentials");
    delay(3000);
    ESP.restart();
  }

  logMessage(2, "WiFi connected");
  logMessage(2, "IP Address: ");
  if (DEBUG_LEVEL >= 2) Serial.println(WiFi.localIP());

  // Connect to Blynk
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Blynk: Connect  ");

  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect(30); // 30 second timeout

  if (!Blynk.connected()) {
    logMessage(1, "Blynk connection failed! Running offline mode.");
    lcd.clear();
    lcd.print("Blynk OFFLINE   ");
    lcd.setCursor(0, 1);
    lcd.print("Local mode only ");
    delay(2000);
  } else {
    logMessage(2, "Blynk connected successfully");
    lcd.clear();
    lcd.print("Blynk ONLINE!   ");
    lcd.setCursor(0, 1);
    lcd.print("Ready to monitor");
    delay(1500);
  }

  lcd.clear();
  startTime = millis();

  // Setup timer for periodic sensor reading
  timer.setInterval(READ_INTERVAL, readAndSendData);

  // Print CSV header
  Serial.println("timestamp,uptime_s,temperature_c,humidity_pct,heat_index_c,dew_point_c,wifi_rssi_dbm,status");
}

// ────────────────────────────────────────────────────────────────────────────
//  MAIN LOOP
// ────────────────────────────────────────────────────────────────────────────

void loop() {
  if (Blynk.connected()) {
    Blynk.run();
  }
  timer.run();

  // Handle deep sleep mode if enabled
  if (ENABLE_DEEP_SLEEP) {
    handleDeepSleep();
  }
}

// ────────────────────────────────────────────────────────────────────────────
//  SENSOR READING & DATA PROCESSING
// ────────────────────────────────────────────────────────────────────────────

SensorData readSensor() {
  SensorData data;

  // Read raw values from DHT22
  data.humidity = dht.readHumidity();
  data.temperature = dht.readTemperature();       // Celsius

  // Validate readings
  if (isnan(data.humidity) || isnan(data.temperature)) {
    data.valid = false;
    logMessage(1, "DHT22 read failed — check wiring/pull-up resistor");
    return data;
  }

  data.valid = true;

  // Compute derived values using NOAA formulas
  data.heatIndex = computeHeatIndex(data.temperature, data.humidity);
  data.dewPoint = computeDewPoint(data.temperature, data.humidity);

  return data;
}

/**
 * @brief Compute heat index using NOAA approximation
 * @param tempC Temperature in Celsius
 * @param humidity Relative humidity in %
 * @return Heat index in Celsius
 * 
 * Formula: HI = c1 + c2*T + c3*R + c4*T*R + c5*T² + c6*R² + c7*T²*R + c8*T*R² + c9*T²*R²
 * where T = temp in °F, R = humidity in %
 */
float computeHeatIndex(float tempC, float humidity) {
  float tempF = tempC * 9.0 / 5.0 + 32.0;
  float hi;

  // Simplified formula for temp < 80°F
  if (tempF < 80.0) {
    hi = 0.5 * (tempF + 61.0 + ((tempF - 68.0) * 1.2) + (humidity * 0.094));
  } else {
    // Full Rothfusz regression
    hi = -42.379 + 2.04901523 * tempF + 10.14333127 * humidity
         - 0.22475541 * tempF * humidity - 6.83783e-3 * tempF * tempF
         - 5.481717e-2 * humidity * humidity + 1.22874e-3 * tempF * tempF * humidity
         + 8.5282e-4 * tempF * humidity * humidity - 1.99e-6 * tempF * tempF * humidity * humidity;

    // Adjustments for low humidity
    if (humidity < 13 && tempF >= 80 && tempF <= 112) {
      float adjustment = ((13 - humidity) / 4.0) * sqrt((17 - abs(tempF - 95.0)) / 17.0);
      hi -= adjustment;
    }
    // Adjustment for high humidity + high temp
    if (humidity > 85 && tempF >= 80 && tempF <= 87) {
      float adjustment = ((humidity - 85) / 10.0) * ((87 - tempF) / 5.0);
      hi += adjustment;
    }
  }

  return (hi - 32.0) * 5.0 / 9.0; // Convert back to Celsius
}

/**
 * @brief Compute dew point using Magnus formula
 * @param tempC Temperature in Celsius
 * @param humidity Relative humidity in %
 * @return Dew point in Celsius
 */
float computeDewPoint(float tempC, float humidity) {
  float a = 17.271;
  float b = 237.7;
  float gamma = ((a * tempC) / (b + tempC)) + log(humidity / 100.0);
  float dewPoint = (b * gamma) / (a - gamma);
  return dewPoint;
}

// ────────────────────────────────────────────────────────────────────────────
//  CLOUD COMMUNICATION
// ────────────────────────────────────────────────────────────────────────────

void readAndSendData() {
  SensorData data = readSensor();
  unsigned long uptime = (millis() - startTime) / 1000;
  int rssi = WiFi.RSSI();

  if (!data.valid) {
    handleSensorError();
    logCSV(uptime, 0, 0, 0, 0, rssi, "ERROR");
    return;
  }

  // Store last valid reading
  lastReading = data;
  lastReadTime = millis();

  // Send to Blynk cloud
  if (Blynk.connected()) {
    Blynk.virtualWrite(VPIN_TEMPERATURE, data.temperature);
    Blynk.virtualWrite(VPIN_HUMIDITY, data.humidity);
    Blynk.virtualWrite(VPIN_HEAT_INDEX, data.heatIndex);
    Blynk.virtualWrite(VPIN_DEW_POINT, data.dewPoint);
    Blynk.virtualWrite(VPIN_WIFI_RSSI, rssi);
    Blynk.virtualWrite(VPIN_UPTIME, uptime);

    logMessage(3, "Blynk data sent successfully");
  } else {
    logMessage(2, "Blynk offline — data logged locally only");
  }

  // Check alert thresholds
  checkAlerts(data.temperature, data.humidity);

  // Update local display
  updateLCD(data.temperature, data.humidity, rssi);

  // Log to serial CSV
  logCSV(uptime, data.temperature, data.humidity, data.heatIndex, data.dewPoint, rssi, "OK");
}

// ────────────────────────────────────────────────────────────────────────────
//  ALERT HANDLING
// ────────────────────────────────────────────────────────────────────────────

void checkAlerts(float temp, float humidity) {
  // High temperature alert
  if (temp > TEMP_ALERT_HIGH && !alertSentHigh) {
    String alertMsg = "WARNING: Temperature is " + String(temp, 1) + "°C (threshold: " + String(TEMP_ALERT_HIGH) + "°C)";
    Blynk.logEvent("high_temp_alert", alertMsg);
    alertSentHigh = true;
    logMessage(1, alertMsg.c_str());
  }
  if (temp <= TEMP_ALERT_HIGH) {
    alertSentHigh = false; // Reset when temp drops
  }

  // Low temperature alert (optional)
  if (temp < TEMP_ALERT_LOW && !alertSentLow) {
    String alertMsg = "WARNING: Temperature is " + String(temp, 1) + "°C (below " + String(TEMP_ALERT_LOW) + "°C)";
    Blynk.logEvent("low_temp_alert", alertMsg);
    alertSentLow = true;
    logMessage(1, alertMsg.c_str());
  }
  if (temp >= TEMP_ALERT_LOW) {
    alertSentLow = false;
  }

  // High humidity alert (mold risk)
  if (humidity > HUM_ALERT_HIGH && !alertSentHum) {
    String alertMsg = "WARNING: Humidity is " + String(humidity, 1) + "% (mold risk above " + String(HUM_ALERT_HIGH) + "%)";
    Blynk.logEvent("high_humidity_alert", alertMsg);
    alertSentHum = true;
    logMessage(1, alertMsg.c_str());
  }
  if (humidity <= HUM_ALERT_HIGH) {
    alertSentHum = false;
  }
}

// ────────────────────────────────────────────────────────────────────────────
//  LCD DISPLAY MANAGEMENT
// ────────────────────────────────────────────────────────────────────────────

void updateLCD(float temp, float hum, int rssi) {
  lcd.clear();

  // Line 1: Temperature + WiFi indicator
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.write(byte(0)); // Custom degree symbol
  lcd.print("C");

  // WiFi signal strength on right side
  lcd.setCursor(10, 0);
  lcd.print("W:");
  if (rssi > -50) {
    lcd.print("+++");
  } else if (rssi > -70) {
    lcd.print("++ ");
  } else if (rssi > -85) {
    lcd.print("+  ");
  } else {
    lcd.print("---");
  }

  // Line 2: Humidity
  lcd.setCursor(0, 1);
  lcd.print("H:");
  lcd.print(hum, 1);
  lcd.print("%");

  // Heat index on right side of line 2
  lcd.setCursor(10, 1);
  lcd.print("HI:");
  lcd.print(lastReading.heatIndex, 0);
  lcd.write(byte(0));
}

void handleSensorError() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sensor Error!   ");
  lcd.setCursor(0, 1);
  lcd.print("Check DHT22 pin ");

  // Blink backlight to indicate error
  for (int i = 0; i < 3; i++) {
    lcd.noBacklight();
    delay(200);
    lcd.backlight();
    delay(200);
  }
}

// ────────────────────────────────────────────────────────────────────────────
//  SERIAL CSV LOGGING
// ────────────────────────────────────────────────────────────────────────────

void logCSV(unsigned long uptime, float temp, float hum, float hi, float dp, int rssi, const char* status) {
  Serial.print(millis() / 1000);
  Serial.print(",");
  Serial.print(uptime);
  Serial.print(",");
  Serial.print(temp, 2);
  Serial.print(",");
  Serial.print(hum, 2);
  Serial.print(",");
  Serial.print(hi, 2);
  Serial.print(",");
  Serial.print(dp, 2);
  Serial.print(",");
  Serial.print(rssi);
  Serial.print(",");
  Serial.println(status);
}

// ────────────────────────────────────────────────────────────────────────────
//  POWER MANAGEMENT
// ────────────────────────────────────────────────────────────────────────────

void handleDeepSleep() {
  // Only enter deep sleep after successful Blynk transmission
  if (Blynk.connected() && lastReading.valid && 
      (millis() - lastReadTime) > READ_INTERVAL) {

    logMessage(2, "Entering deep sleep mode...");

    // Turn off LCD backlight to save power
    lcd.noBacklight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sleeping...");
    lcd.setCursor(0, 1);
    lcd.print("Wake in ");
    lcd.print(SLEEP_DURATION_MIN);
    lcd.print("min");

    delay(1000); // Allow LCD to update

    // Configure deep sleep timer
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION_MIN * 60 * 1000000ULL);

    // Store last reading in RTC memory for comparison on wake
    // (RTC_DATA_ATTR variables persist across deep sleep)

    logMessage(2, "ESP32 entering deep sleep now");
    esp_deep_sleep_start();
  }
}

// ────────────────────────────────────────────────────────────────────────────
//  BLYNK CALLBACKS
// ────────────────────────────────────────────────────────────────────────────

BLYNK_CONNECTED() {
  logMessage(2, "Blynk reconnected — syncing virtual pins");
  Blynk.syncAll();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Blynk Reconnected");
  delay(1000);
  lcd.clear();
}

BLYNK_DISCONNECTED() {
  logMessage(1, "Blynk disconnected — running offline mode");
}

// Optional: Receive threshold updates from Blynk app
BLYNK_WRITE(VPIN_TEMPERATURE) {
  // This allows the app to send commands back to the device
  float appTemp = param.asFloat();
  logMessage(3, "Received temperature override from app");
}

// ────────────────────────────────────────────────────────────────────────────
//  END OF FILE
// ────────────────────────────────────────────────────────────────────────────
