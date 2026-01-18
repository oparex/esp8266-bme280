# ESP8266 BME280 Sensor → MQTT

A lightweight ESP8266 project that reads temperature, humidity, and pressure from a BME280 sensor and publishes the data to an MQTT broker in JSON format. Designed for integration with Telegraf and QuestDB for time-series data storage and visualization.

## Features

- **BME280 Sensor Integration**: Reads temperature (°C), humidity (%), and pressure (hPa)
- **MQTT Publishing**: Sends sensor data as JSON to a configurable MQTT broker
- **WiFi Connectivity**: Connects to your local WiFi network
- **Last Will and Testament (LWT)**: Publishes device online/offline status
- **Optional Deep Sleep**: Ultra-low power mode (~50 µA) for battery-powered deployments
- **Configurable Interval**: Adjustable publish frequency (default: 60 seconds)
- **JSON Format**: Clean, structured data output with ArduinoJson

## Hardware Requirements

- **ESP8266** (NodeMCU, Wemos D1 Mini, or similar)
- **BME280** sensor module (I2C)
- Connecting wires
- Power supply (USB or battery)

## Wiring

Connect the BME280 sensor to your ESP8266:

| BME280 Pin | ESP8266 Pin |
|------------|-------------|
| VCC        | 3.3V        |
| GND        | GND         |
| SCL        | D1 (GPIO5)  |
| SDA        | D2 (GPIO4)  |

> **Note**: The BME280 typically uses I2C address `0x76`. If your sensor uses `0x77`, modify line 37 in `main.ino`.

## Software Dependencies

Install the following libraries via Arduino IDE Library Manager:

1. **ESP8266WiFi** (included with ESP8266 board package)
2. **PubSubClient** by Nick O'Leary
3. **Adafruit Unified Sensor**
4. **Adafruit BME280 Library**
5. **ArduinoJson** by Benoit Blanchon (v6.x)

### ESP8266 Board Setup

1. In Arduino IDE, go to **File → Preferences**
2. Add to "Additional Board Manager URLs":
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. Go to **Tools → Board → Boards Manager**
4. Search for "esp8266" and install **ESP8266 by ESP8266 Community**

## Configuration

### 1. Edit `secrets.h`

Configure your WiFi and MQTT settings:

```cpp
// WiFi credentials
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

// MQTT broker settings
const char* mqtt_server = "192.168.1.100";  // Your MQTT broker IP
const int   mqtt_port = 1883;
const char* mqtt_client_id = "esp8266-bedroom-bme280";

// Optional MQTT authentication
const char* mqtt_username = "";  // Leave empty if no auth
const char* mqtt_password = "";

// Device identification
const char* device_id = "bme280_01";
const char* device_location = "bedroom";

// MQTT topics
const char* mqtt_topic = "sensors/bedroom/bme280";
const char* mqtt_lwt_topic = "sensors/bedroom/bme280/status";
```

### 2. Adjust Settings in `main.ino`

```cpp
// Publish interval (seconds)
const uint64_t publish_interval_sec = 60;  // 60-300 typical for home use

// Enable deep sleep (uncomment for battery power)
// #define USE_DEEP_SLEEP
```

## Installation

1. Clone or download this repository
2. Open `main.ino` in Arduino IDE
3. Configure `secrets.h` with your WiFi and MQTT settings
4. Select your ESP8266 board: **Tools → Board → ESP8266 Boards → [Your Board]**
5. Select the correct port: **Tools → Port → [Your COM Port]**
6. Upload the sketch

## Data Format

The device publishes JSON messages to the configured MQTT topic:

```json
{
  "temperature": 22.5,
  "humidity": 45.3,
  "pressure": 1013.2,
  "location": "bedroom",
  "device_id": "bme280_01"
}
```

### Field Descriptions

- `temperature`: Temperature in Celsius (1 decimal place)
- `humidity`: Relative humidity percentage (1 decimal place)
- `pressure`: Atmospheric pressure in hPa (1 decimal place)
- `location`: Device location (from `secrets.h`)
- `device_id`: Unique device identifier (from `secrets.h`)

## Deep Sleep Mode

For battery-powered deployments, enable deep sleep by uncommenting in `main.ino`:

```cpp
#define USE_DEEP_SLEEP
```

**Important**: When using deep sleep:
- Connect GPIO16 (D0) to RST pin to allow automatic wake-up
- Current consumption drops to ~50 µA during sleep
- The device wakes up, takes a reading, publishes, then sleeps again

## Integration Examples

### Telegraf Configuration

```toml
[[inputs.mqtt_consumer]]
  servers = ["tcp://localhost:1883"]
  topics = ["sensors/+/bme280"]
  data_format = "json"
  tag_keys = ["location", "device_id"]
```

### QuestDB Ingestion

Telegraf can forward data to QuestDB for time-series storage:

```toml
[[outputs.socket_writer]]
  address = "tcp://localhost:9009"
  data_format = "influx"
```

### Home Assistant

```yaml
sensor:
  - platform: mqtt
    name: "Bedroom Temperature"
    state_topic: "sensors/bedroom/bme280"
    unit_of_measurement: "°C"
    value_template: "{{ value_json.temperature }}"
```

## Troubleshooting

### BME280 Not Found

- Verify I2C wiring (SDA/SCL connections)
- Try changing the I2C address from `0x76` to `0x77` in line 37
- Test with I2C scanner sketch

### WiFi Connection Issues

- Double-check SSID and password in `secrets.h`
- Ensure 2.4GHz WiFi (ESP8266 doesn't support 5GHz)
- Check signal strength

### MQTT Not Connecting

- Verify MQTT broker IP address and port
- Check firewall settings
- Test broker with mosquitto_sub: `mosquitto_sub -h [broker-ip] -t sensors/#`
- If using authentication, ensure username/password are correct

### No Data Published

- Uncomment Serial debug lines in the code to see diagnostic messages
- Check MQTT topic subscription: `mosquitto_sub -h [broker-ip] -t sensors/# -v`
- Verify publish interval hasn't been set too long

## Power Consumption

- **Active mode**: ~70-80 mA (WiFi on, sensing and publishing)
- **Deep sleep mode**: ~50 µA (between readings)
- **Typical battery life** (with deep sleep, 60s interval):
  - 2000 mAh battery: ~6-12 months
  - 18650 Li-Ion (3000 mAh): ~9-18 months

## License

This project is open source. Feel free to modify and adapt for your needs.

## Data Pipeline Architecture

```
ESP8266 + BME280
    ↓ (MQTT/JSON)
Mosquitto MQTT Broker
    ↓
Telegraf
    ↓
QuestDB / InfluxDB
    ↓
Grafana / Dashboard
```

## Contributing

Issues and pull requests are welcome! Feel free to improve the code or documentation.
