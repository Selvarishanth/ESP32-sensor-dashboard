/*
 * test_sensor_validation.ino
 * 
 * DHT22 Sensor Accuracy Validation Test
 * 
 * Run this sketch for 24 hours alongside a calibrated reference thermometer
 * to validate DHT22 accuracy and reliability.
 */

#include <DHT.h>

#define DHT_PIN  4
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

unsigned long testStartTime = 0;
int totalReads = 0;
int successfulReads = 0;
int failedReads = 0;
float tempSum = 0;
float humSum = 0;
float tempMin = 100;
float tempMax = -100;
float humMin = 100;
float humMax = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  testStartTime = millis();

  Serial.println("=== DHT22 Sensor Validation Test ===");
  Serial.println("Place calibrated reference thermometer next to DHT22");
  Serial.println("Run for 24 hours minimum for reliable statistics");
  Serial.println();
  Serial.println("timestamp,reference_temp,dht_temp,dht_humidity,status");
}

void loop() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  totalReads++;

  if (isnan(hum) || isnan(temp)) {
    failedReads++;
    Serial.print(millis() / 1000);
    Serial.println(",nan,nan,nan,FAIL");
  } else {
    successfulReads++;
    tempSum += temp;
    humSum += hum;
    tempMin = min(tempMin, temp);
    tempMax = max(tempMax, temp);
    humMin = min(humMin, hum);
    humMax = max(humMax, hum);

    Serial.print(millis() / 1000);
    Serial.print(",");
    Serial.print(temp, 2);  // Replace with reference reading
    Serial.print(",");
    Serial.print(temp, 2);
    Serial.print(",");
    Serial.print(hum, 2);
    Serial.println(",OK");
  }

  // Print statistics every 100 reads
  if (totalReads % 100 == 0) {
    printStats();
  }

  delay(2000);  // DHT22 minimum interval
}

void printStats() {
  float successRate = (float)successfulReads / totalReads * 100;
  float avgTemp = tempSum / successfulReads;
  float avgHum = humSum / successfulReads;

  Serial.println();
  Serial.println("=== STATISTICS ===");
  Serial.print("Total reads: "); Serial.println(totalReads);
  Serial.print("Success rate: "); Serial.print(successRate); Serial.println("%");
  Serial.print("Avg Temp: "); Serial.print(avgTemp, 2); Serial.println("°C");
  Serial.print("Temp Range: "); Serial.print(tempMin, 2); Serial.print(" - "); Serial.print(tempMax, 2); Serial.println("°C");
  Serial.print("Avg Hum: "); Serial.print(avgHum, 2); Serial.println("%");
  Serial.print("Hum Range: "); Serial.print(humMin, 2); Serial.print(" - "); Serial.print(humMax, 2); Serial.println("%");
  Serial.println("==================");
  Serial.println();
}
