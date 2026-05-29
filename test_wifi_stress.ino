/*
 * test_wifi_stress_test.ino
 * 
 * WiFi Connection Reliability Test
 * 
 * Tests ESP32 WiFi reconnection behavior under various conditions.
 */

#include <WiFi.h>

const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";

unsigned long testStartTime = 0;
int connectionAttempts = 0;
int successfulConnections = 0;
int disconnections = 0;
unsigned long totalConnectedTime = 0;
unsigned long lastConnectTime = 0;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  testStartTime = millis();

  Serial.println("=== WiFi Stress Test ===");
  Serial.println("This test will connect/disconnect WiFi 50 times");
  Serial.println("timestamp,event,rssi,connect_time_ms,status");
}

void loop() {
  static int testCount = 0;
  static bool connected = false;

  if (testCount >= 50) {
    printFinalStats();
    while(true) delay(1000);  // Stop after 50 cycles
  }

  if (!connected) {
    // Attempt connection
    connectionAttempts++;
    unsigned long connectStart = millis();
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
      delay(500);
      attempts++;
    }

    unsigned long connectTime = millis() - connectStart;

    if (WiFi.status() == WL_CONNECTED) {
      successfulConnections++;
      connected = true;
      lastConnectTime = millis();

      Serial.print(millis() / 1000);
      Serial.print(",CONNECT,");
      Serial.print(WiFi.RSSI());
      Serial.print(",");
      Serial.print(connectTime);
      Serial.println(",SUCCESS");
    } else {
      Serial.print(millis() / 1000);
      Serial.print(",CONNECT,0,");
      Serial.print(connectTime);
      Serial.println(",FAIL");
    }
  } else {
    // Stay connected for 10 seconds, then disconnect
    delay(10000);

    totalConnectedTime += (millis() - lastConnectTime);
    disconnections++;

    Serial.print(millis() / 1000);
    Serial.print(",DISCONNECT,");
    Serial.print(WiFi.RSSI());
    Serial.print(",10000,OK");
    Serial.println();

    WiFi.disconnect();
    connected = false;
    testCount++;

    delay(2000);  // Wait before next connection attempt
  }
}

void printFinalStats() {
  float successRate = (float)successfulConnections / connectionAttempts * 100;
  float avgConnectTime = totalConnectedTime / successfulConnections;

  Serial.println();
  Serial.println("=== FINAL STATISTICS ===");
  Serial.print("Total attempts: "); Serial.println(connectionAttempts);
  Serial.print("Successful: "); Serial.println(successfulConnections);
  Serial.print("Success rate: "); Serial.print(successRate); Serial.println("%");
  Serial.print("Disconnections: "); Serial.println(disconnections);
  Serial.print("Avg connected time: "); Serial.print(avgConnectTime / 1000); Serial.println("s");
  Serial.println("========================");
}
