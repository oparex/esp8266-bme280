// secrets.h
#ifndef SECRETS_H
#define SECRETS_H

// ---- Wi-Fi -------------------------------------------------
const char* ssid = "42";
const char* password = "";

// ---- QuestDB -----------------------------------------------
const char* mqtt_server = "10.0.0.252";  // Your laptop's IP
const int   mqtt_port = 1883;
const char* mqtt_client_id      = "esp8266-miningroom-bme280";
const char* mqtt_username       = "";       // leave empty "" if no auth
const char* mqtt_password  = "";      // leave empty "" if no auth

const char* device_id = "bme280_01";
const char* device_location = "miningroom";

const char* mqtt_topic          = "sensors/mining/bme280";
// Optional Last-Will topic (shows device offline in Home Assistant, Node-RED, etc.)
const char* mqtt_lwt_topic      = "sensors/miningroom/bme280/status";

#endif
