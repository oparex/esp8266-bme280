/**************************************************************
  ESP8266 + BME280 → MQTT (Mosquitto) → Telegraf → QuestDB
  Single JSON message per reading
  Deep-sleep capable (remove if you want it always on)
**************************************************************/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>

// Load secrets from separate file
#include "secrets.h"

// ------------------- CONFIGURATION -------------------
// How often to read and publish (seconds). 60–300 is typical for home use.
const uint64_t publish_interval_sec = 60;

// Deep sleep (uncomment next line if you want ultra-low power ~50 µA)
// #define USE_DEEP_SLEEP

// -----------------------------------------------------

Adafruit_BME280 bme;
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPublish = 0;

void setup() {
  // Serial.begin(115200);
  // delay(100);

  // ---- BME280 init ----
  if (!bme.begin(0x76)) {               // 0x76 is most common address, try 0x77 if it fails
    // Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    while (1) delay(10);
  }

  // ---- Wi-Fi ----
  setupWiFi();

  // ---- MQTT ----
  client.setServer(mqtt_server, mqtt_port);
  reconnectMQTT();   // will loop until connected
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastPublish > (publish_interval_sec * 1000UL) || lastPublish == 0) {
    lastPublish = now;
    readAndPublishBME();
  }

#ifndef USE_DEEP_SLEEP
  // If you don't use deep sleep, just delay a bit so loop doesn't spin too fast
  delay(1000);
#endif
}

// -----------------------------------------------------
void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // Serial.print(F("Connecting to WiFi"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // Serial.print(".");
  }
  // Serial.println();
  // Serial.print(F("Connected, IP: "));
  // Serial.println(WiFi.localIP());
}

// -----------------------------------------------------
void reconnectMQTT() {
  while (!client.connected()) {
    // Serial.print(F("Attempting MQTT connection..."));
    bool success;
    if (strlen(mqtt_username) > 0) {
      success = client.connect(mqtt_client_id, mqtt_username, mqtt_password, mqtt_lwt_topic, 0, true, "offline");
    } else {
      success = client.connect(mqtt_client_id, mqtt_lwt_topic, 0, true, "offline");
    }

    if (success) {
      // Serial.println(F("connected"));
      // Publish online status (LWT = online)
      client.publish(mqtt_lwt_topic, "online", true);
    } else {
      // Serial.print(F("failed, rc="));
      // Serial.print(client.state());
      // Serial.println(F(" try again in 5 seconds"));
      delay(5000);
    }
  }
}

// -----------------------------------------------------
void readAndPublishBME() {
  float temp = bme.readTemperature();          // °C
  float hum  = bme.readHumidity();               // %
  float pres = bme.readPressure() / 100.0F;       // hPa

  // Build nice JSON with ArduinoJson 6
  StaticJsonDocument<200> doc;
  doc["temperature"] = roundf(temp * 10) / 10.0;   // 1 decimal
  doc["humidity"]    = roundf(hum  * 10) / 10.0;
  doc["pressure"]    = roundf(pres * 10) / 10.0;
  doc["location"]    = device_location;
  doc["device_id"]   = device_id;

  char buffer[256];
  size_t n = serializeJson(doc, buffer);

  // bool published = client.publish(mqtt_topic, buffer, n);
  // Serial.print(F("Publish "));
  // Serial.print(mqtt_topic);
  // Serial.print(F(" → "));
  // Serial.println(published ? F("OK") : F("FAILED"));
  client.publish(mqtt_topic, buffer, n);
  client.loop();  // flush

#ifdef USE_DEEP_SLEEP
  // Serial.println(F("Going into deep sleep..."));
  ESP.deepSleep(publish_interval_sec * 1000000ULL, WAKE_RF_DEFAULT);
  delay(100); // give serial time to finish
#endif
}
