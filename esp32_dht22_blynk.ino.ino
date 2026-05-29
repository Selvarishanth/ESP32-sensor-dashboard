/*
 * ============================================================
 *  ESP32 WiFi Temperature & Humidity Dashboard
 *  Sensor  : DHT22 (AM2302)
 *  Platform: Blynk IoT (New Blynk — blynk.cloud)
 *  Display : 16x2 I2C LCD (optional, remove if not using)
 *
 *  What it does:
 *    - Reads temperature + humidity from DHT22 every 2 seconds
 *    - Shows live values on Blynk app (phone dashboard)
 *    - Shows values on LCD display (if connected)
 *    - Sends alert notification if temp exceeds threshold
 *    - Prints CSV data on Serial Monitor for logging
 *
 *  WIRING:
 *    DHT22 Pin 1 (VCC)  → 3.3V
 *    DHT22 Pin 2 (DATA) → GPIO 4  (with 10kΩ pull-up to 3.3V)
 *    DHT22 Pin 4 (GND)  → GND
 *
 *    LCD I2C (optional):
 *    VCC → 5V (or 3.3V if your module supports it)
 *    GND → GND
 *    SDA → GPIO 21
 *    SCL → GPIO 22
 *
 *  LIBRARIES (install via Arduino IDE → Manage Libraries):
 *    1. DHT sensor library       by Adafruit
 *    2. Adafruit Unified Sensor  by Adafruit
 *    3. Blynk                    by Volodymyr Shymanskyy  v1.3.2+
 *    4. LiquidCrystal_I2C        by Frank de Brabander (optional)
 *
 *  SETUP STEPS:
 *    1. Create account at blynk.cloud
 *    2. New Template → name it "DHT22 Dashboard"
 *    3. Add Datastreams:
 *         V0 → Temperature (°C), double, -40 to 80
 *         V1 → Humidity (%),     double,   0 to 100
 *         V2 → Heat Index (°C),  double, -40 to 80
 *    4. Create Dashboard → add Gauge/Chart widgets on V0, V1, V2
 *    5. Get your BLYNK_TEMPLATE_ID, BLYNK_TEMPLATE_NAME, BLYNK_AUTH_TOKEN
 *    6. Fill them in below
 *
 *  Author : Selverishanth, SRM Institute
 *  Course : 21EIC101J – Sensors and Actuators
 * ============================================================
 */

// ── Blynk credentials (fill these in from blynk.cloud) ──────
#define BLYNK_TEMPLATE_ID    "Your temp ID"      // ← replace
#define BLYNK_TEMPLATE_NAME  "Your Dashboard name"
#define BLYNK_AUTH_TOKEN     "Your auth token"   // ← replace

#define BLYNK_PRINT Serial   // show Blynk debug on Serial Monitor

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ── WiFi credentials ─────────────────────────────────────────
const char* WIFI_SSID     = "Your_WiFi_Name";     // ← replace
const char* WIFI_PASSWORD = "Your_WiFi_Password"; // ← replace

// ── DHT22 setup ──────────────────────────────────────────────
#define DHT_PIN  4           // GPIO4 — DATA pin of DHT22
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// ── LCD setup (I2C address 0x27 — try 0x3F if blank) ────────
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ── Blynk Virtual Pins ───────────────────────────────────────
#define VPIN_TEMP        V0
#define VPIN_HUMIDITY    V1
#define VPIN_HEAT_INDEX  V2

// ── Alert threshold ──────────────────────────────────────────
#define TEMP_ALERT_C    35.0   // send Blynk notification above this

// ── Timing ───────────────────────────────────────────────────
#define READ_INTERVAL_MS  2000  // read sensor every 2 seconds

// ── Globals ──────────────────────────────────────────────────
BlynkTimer timer;
float lastTemp      = 0;
float lastHumidity  = 0;
float lastHeatIndex = 0;
bool  alertSent     = false;

// Custom LCD degree symbol
byte degreeChar[8] = {
  0b01100, 0b10010, 0b10010,
  0b01100, 0b00000, 0b00000,
  0b00000, 0b00000
};

// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32 DHT22 Blynk Dashboard ===");

  // Start DHT sensor
  dht.begin();

  // Start LCD
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, degreeChar);

  // Splash screen
  lcd.setCursor(0, 0); lcd.print("  Temp & Humid  ");
  lcd.setCursor(0, 1); lcd.print("  Dashboard...  ");
  delay(1500);
  lcd.clear();

  // Connect to WiFi + Blynk
  lcd.setCursor(0, 0); lcd.print("Connecting WiFi ");
  lcd.setCursor(0, 1); lcd.print(WIFI_SSID);

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);

  // Show connected
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("WiFi Connected! ");
  lcd.setCursor(0, 1); lcd.print(WiFi.localIP());
  delay(2000);
  lcd.clear();

  // Print IP on Serial
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Set timer to read sensor every 2 seconds
  timer.setInterval(READ_INTERVAL_MS, readAndSend);

  Serial.println("Time(s), Temp(C), Humidity(%), HeatIndex(C)");
}

// ════════════════════════════════════════════════════════════
void loop() {
  Blynk.run();    // handles Blynk connection + data
  timer.run();    // handles timed sensor reads
}

// ── Read DHT22 and send to Blynk ─────────────────────────────
void readAndSend() {

  float humidity   = dht.readHumidity();
  float tempC      = dht.readTemperature();       // Celsius
  float heatIndex  = dht.computeHeatIndex(tempC, humidity, false);

  // Check for sensor read failure
  if (isnan(humidity) || isnan(tempC)) {
    Serial.println("ERROR: DHT22 read failed. Check wiring.");
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Sensor Error!   ");
    lcd.setCursor(0, 1); lcd.print("Check DHT22 pin ");
    return;
  }

  lastTemp      = tempC;
  lastHumidity  = humidity;
  lastHeatIndex = heatIndex;

  // ── Send to Blynk app ─────────────────────────────────────
  Blynk.virtualWrite(VPIN_TEMP,       tempC);
  Blynk.virtualWrite(VPIN_HUMIDITY,   humidity);
  Blynk.virtualWrite(VPIN_HEAT_INDEX, heatIndex);

  // ── Temperature alert (once per high event) ───────────────
  if (tempC > TEMP_ALERT_C && !alertSent) {
    Blynk.logEvent("high_temp_alert",
      String("WARNING: Temperature is ") + tempC + "°C");
    alertSent = true;
    Serial.println("ALERT sent: Temperature too high!");
  }
  if (tempC <= TEMP_ALERT_C) {
    alertSent = false;  // reset when temp drops back
  }

  // ── Update LCD ────────────────────────────────────────────
  updateLCD(tempC, humidity);

  // ── Serial CSV log ────────────────────────────────────────
  Serial.print(millis() / 1000);
  Serial.print(", ");
  Serial.print(tempC, 1);
  Serial.print(", ");
  Serial.print(humidity, 1);
  Serial.print(", ");
  Serial.println(heatIndex, 1);
}

// ── Update 16x2 LCD display ──────────────────────────────────
void updateLCD(float temp, float hum) {
  lcd.clear();

  // Line 1: T:xx.x°C
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.write(byte(0));   // degree symbol
  lcd.print("C");

  // Line 2: H:xx.x%
  lcd.setCursor(0, 1);
  lcd.print("H:");
  lcd.print(hum, 1);
  lcd.print("%");

  // WiFi signal strength on right side of line 1
  int rssi = WiFi.RSSI();
  lcd.setCursor(10, 0);
  if      (rssi > -50) lcd.print("WiFi:+++");
  else if (rssi > -70) lcd.print("WiFi:++ ");
  else                 lcd.print("WiFi:+  ");
}

// ── Blynk connected callback ─────────────────────────────────
BLYNK_CONNECTED() {
  Serial.println("Blynk connected!");
  // Sync all virtual pins from server on reconnect
  Blynk.syncAll();
}
