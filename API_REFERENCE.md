# API Reference

## Blynk Virtual Pin API

### Datastreams (Device → Cloud)

#### V0 — Temperature
- **Name**: `temperature`
- **Type**: `Double`
- **Range**: -40.0 to 80.0 °C
- **Precision**: 1 decimal place
- **Update Rate**: Every 2 seconds
- **Widget**: Gauge, Chart, Label

#### V1 — Humidity
- **Name**: `humidity`
- **Type**: `Double`
- **Range**: 0.0 to 100.0 %
- **Precision**: 1 decimal place
- **Update Rate**: Every 2 seconds
- **Widget**: Gauge, Chart, Label

#### V2 — Heat Index
- **Name**: `heat_index`
- **Type**: `Double`
- **Range**: -40.0 to 80.0 °C
- **Precision**: 1 decimal place
- **Update Rate**: Every 2 seconds
- **Widget**: Gauge, Chart

#### V3 — Dew Point
- **Name**: `dew_point`
- **Type**: `Double`
- **Range**: -40.0 to 80.0 °C
- **Precision**: 1 decimal place
- **Update Rate**: Every 2 seconds
- **Widget**: Label, Chart

#### V4 — WiFi RSSI
- **Name**: `wifi_rssi`
- **Type**: `Integer`
- **Range**: -100 to 0 dBm
- **Update Rate**: Every 2 seconds
- **Widget**: Label, LED
- **Interpretation**:
  - -30 to -50 dBm: Excellent
  - -50 to -70 dBm: Good
  - -70 to -85 dBm: Fair
  - Below -85 dBm: Poor

#### V5 — Uptime
- **Name**: `uptime`
- **Type**: `Integer`
- **Range**: 0 to 86400 seconds
- **Update Rate**: Every 2 seconds
- **Widget**: Label

### Events (Cloud → Device)

#### high_temp_alert
- **Trigger**: Temperature > 35.0°C
- **Type**: Push notification
- **Message**: `"WARNING: Temperature is {temp}°C (threshold: 35°C)"`
- **Rate Limit**: Once per high-temperature event (resets when temp drops)

#### low_temp_alert
- **Trigger**: Temperature < 10.0°C
- **Type**: Push notification
- **Message**: `"WARNING: Temperature is {temp}°C (below 10°C)"`

#### high_humidity_alert
- **Trigger**: Humidity > 85.0%
- **Type**: Push notification
- **Message**: `"WARNING: Humidity is {hum}% (mold risk above 85%)"`

## Serial CSV API

### Output Format
```csv
timestamp,uptime_s,temperature_c,humidity_pct,heat_index_c,dew_point_c,wifi_rssi_dbm,status
```

### Example Output
```
1625,1625,28.50,65.20,31.20,21.40,-52,OK
1627,1627,28.55,65.15,31.25,21.35,-53,OK
1629,1629,nan,nan,nan,nan,-52,ERROR
```

### Fields
| Field | Type | Description |
|-------|------|-------------|
| `timestamp` | Integer | Unix timestamp (seconds since boot) |
| `uptime_s` | Integer | Total system uptime in seconds |
| `temperature_c` | Float | Temperature in Celsius (or `nan` if error) |
| `humidity_pct` | Float | Relative humidity % (or `nan` if error) |
| `heat_index_c` | Float | Computed heat index in Celsius |
| `dew_point_c` | Float | Computed dew point in Celsius |
| `wifi_rssi_dbm` | Integer | WiFi signal strength in dBm |
| `status` | String | `OK` or `ERROR` |

## Firmware Functions

### `SensorData readSensor()`
Reads and validates DHT22 data.
- **Returns**: `SensorData` struct with temperature, humidity, heatIndex, dewPoint, valid
- **Error Handling**: Returns `valid=false` if `isnan()` detected

### `float computeHeatIndex(float tempC, float humidity)`
Calculates heat index using NOAA Rothfusz regression.
- **Input**: Temperature (°C), Humidity (%)
- **Output**: Heat index (°C)
- **Accuracy**: ±1.3°C for T > 27°C and RH > 40%

### `float computeDewPoint(float tempC, float humidity)`
Calculates dew point using Magnus formula.
- **Input**: Temperature (°C), Humidity (%)
- **Output**: Dew point (°C)
- **Accuracy**: ±0.4°C for T > 0°C

### `void updateLCD(float temp, float hum, int rssi)`
Updates LCD display with formatted readings.
- **Line 1**: `T:xx.x°C W:+++`
- **Line 2**: `H:xx.x% HI:xx°`

### `void checkAlerts(float temp, float humidity)`
Evaluates thresholds and triggers Blynk events.
- **State Machine**: Tracks `alertSent` flags to prevent spam
- **Reset Condition**: Value returns below threshold
