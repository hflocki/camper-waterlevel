#include "mqtt_ha.h"
#include "sensor.h"
#include "wifi_manager.h"
#include "webserver.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

static WiFiClient espClient;
static PubSubClient mqtt(espClient);

static unsigned long lastPublish = 0;
static unsigned long lastReconnect = 0;
static bool discoveryPublished = false;

static void mqttCallback(char* topic, byte* payload, unsigned int length);
static void publishSensor(const char* name, const char* id, const char* unit,
                          const char* devClass, const char* icon, const char* valueTpl);
static void publishSwitch(const char* name, const char* id, const char* icon, const char* cmdTopic);
static void addDeviceBlock(JsonDocument& doc, bool withAvailability = true);

void mqttInit() {
    if (strlen(config.mqtt_server) == 0) return;

    mqtt.setServer(config.mqtt_server, config.mqtt_port);
    mqtt.setCallback(mqttCallback);
    mqtt.setBufferSize(1024);
    mqtt.setKeepAlive(30);
    mqtt.setSocketTimeout(5);

    Serial.printf("[MQTT] Configured: %s:%d\n", config.mqtt_server, config.mqtt_port);
}

void mqttLoop() {
    if (!isConnected()) return;
    if (strlen(config.mqtt_server) == 0) return;

    if (!mqtt.connected()) {
        // connect() blocks while the broker is unreachable, so retry only every 5 s
        if (millis() - lastReconnect > 5000) {
            lastReconnect = millis();
            Serial.println("[MQTT] Connection attempt...");

            String clientId = "camper-waterlevel-" + WiFi.macAddress();
            clientId.replace(":", "");

            String willTopic = String(config.mqtt_prefix) + "/status";
            bool connected;

            if (strlen(config.mqtt_user) > 0) {
                connected = mqtt.connect(clientId.c_str(), config.mqtt_user, config.mqtt_pass,
                                         willTopic.c_str(), 0, true, "offline");
            } else {
                connected = mqtt.connect(clientId.c_str(),
                                         NULL, NULL, willTopic.c_str(), 0, true, "offline");
            }

            if (connected) {
                Serial.println("[MQTT] Connected!");

                String cmdBase = String(config.mqtt_prefix) + "/cmd/#";
                mqtt.subscribe(cmdBase.c_str());

                if (!discoveryPublished) {
                    mqttPublishDiscovery();
                    discoveryPublished = true;
                }

                mqtt.publish(willTopic.c_str(), "online", true);
            } else {
                Serial.printf("[MQTT] Failed, rc=%d\n", mqtt.state());
            }
        }
        return;
    }

    mqtt.loop();

    unsigned long mqttInterval = config.mqtt_fast ? MQTT_FAST_MS : MQTT_SLOW_MS;
    if (millis() - lastPublish > mqttInterval) {
        lastPublish = millis();
        mqttPublishState();
    }
}

bool mqttIsConnected() {
    return mqtt.connected();
}

static void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String topicStr = String(topic);
    String payloadStr;
    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }
    payloadStr.trim();

    String cmdPrefix = String(config.mqtt_prefix) + "/cmd/";

    if (topicStr == cmdPrefix + "filter/set") {
        String fLower = payloadStr;
        fLower.toLowerCase();

        bool newState;
        if (fLower == "toggle") {
            newState = !config.filter_active;
        } else {
            newState = (fLower == "on" || fLower == "true" || fLower == "1");
        }

        // Only write to flash if the value really changed (reduces flash wear)
        if (newState != config.filter_active) {
            config.filter_active = newState;
            saveConfig();
        }

        String prefix = String(config.mqtt_prefix);
        mqtt.publish((prefix + "/filter_active").c_str(), config.filter_active ? "ON" : "OFF", true);
    }
}

void mqttPublishState() {
    if (!mqtt.connected()) return;

    String prefix = String(config.mqtt_prefix);
    char buf[16];

    dtostrf(sensorData.avg_voltage, 1, 3, buf);
    mqtt.publish((prefix + "/voltage").c_str(), buf, true);

    dtostrf(sensorData.liters, 1, 1, buf);
    mqtt.publish((prefix + "/liters").c_str(), buf, true);

    dtostrf(sensorData.percent, 1, 0, buf);
    mqtt.publish((prefix + "/percent").c_str(), buf, true);

    itoa(sensorData.rssi, buf, 10);
    mqtt.publish((prefix + "/rssi").c_str(), buf, true);

    mqtt.publish((prefix + "/filter_active").c_str(), config.filter_active ? "ON" : "OFF", true);
    mqtt.publish((prefix + "/status").c_str(), "online", true);
}

void mqttPublishDiscovery() {
    Serial.println("[MQTT] Publishing HA Discovery...");

    publishSensor("Tank Spannung",        "voltage", "V",   "voltage",         "mdi:current-dc",   "{{ value }}");
    publishSensor("Frischwasser Inhalt",  "liters",  "L",   "volume_storage",  "mdi:water",        "{{ value }}");
    // No device_class for percent: "battery" would show a battery icon in Home Assistant
    publishSensor("Frischwasser Prozent", "percent", "%",   "None",            "mdi:water-percent", "{{ value }}");
    publishSensor("WLAN Signal",          "rssi",    "dBm", "signal_strength", "mdi:wifi",         "{{ value }}");
    publishSensor("Status",               "status",  "",    "None",            "mdi:check-circle", "{{ value }}");

    String cmdBase = String(config.mqtt_prefix) + "/cmd/";
    publishSwitch("Glättungsfilter", "filter_active", "mdi:blur", (cmdBase + "filter/set").c_str());

    Serial.println("[MQTT] Discovery complete");
}

static void addDeviceBlock(JsonDocument& doc, bool withAvailability) {
    JsonObject device = doc["device"].to<JsonObject>();
    device["identifiers"][0] = "camper_waterlevel_sensor";
    device["name"] = "Camper Wasserlevel Sensor";
    device["model"] = "ESP32 Water Sensor";
    device["manufacturer"] = "DIY";
    device["sw_version"] = "1.0.0";

    // Level sensors and the status sensor skip availability on purpose, so Home Assistant
    // keeps showing the last known value (and "offline") while the device is switched off
    if (withAvailability) {
        doc["availability_topic"] = String(config.mqtt_prefix) + "/status";
        doc["payload_available"] = "online";
        doc["payload_not_available"] = "offline";
    }
}

static void publishSensor(const char* name, const char* id, const char* unit,
                          const char* devClass, const char* icon, const char* valueTpl) {
    JsonDocument doc;

    String uniqueId = String("camper_water_") + id;
    String stateTopic = String(config.mqtt_prefix) + "/" + id;
    String configTopic = String("homeassistant/sensor/camper_water/") + id + "/config";

    doc["name"] = name;
    doc["unique_id"] = uniqueId;
    doc["state_topic"] = stateTopic;
    doc["value_template"] = valueTpl;
    doc["icon"] = icon;

    if (strlen(unit) > 0) doc["unit_of_measurement"] = unit;
    if (strcmp(devClass, "None") != 0) doc["device_class"] = devClass;
    if (strcmp(id, "status") != 0) doc["state_class"] = "measurement";

    bool keepLastValue = (strcmp(id, "voltage") == 0 || strcmp(id, "liters") == 0 ||
                          strcmp(id, "percent") == 0 || strcmp(id, "status") == 0);
    addDeviceBlock(doc, !keepLastValue);

    char buffer[768];
    size_t len = serializeJson(doc, buffer, sizeof(buffer));
    if (len == 0 || len >= sizeof(buffer)) {
        Serial.printf("[MQTT] Discovery payload for '%s' too large\n", id);
        return;
    }
    mqtt.publish(configTopic.c_str(), buffer, true);
}

static void publishSwitch(const char* name, const char* id, const char* icon,
                          const char* cmdTopic) {
    JsonDocument doc;

    String uniqueId = String("camper_water_") + id + "_switch";
    String stateTopic = String(config.mqtt_prefix) + "/" + id;
    String configTopic = String("homeassistant/switch/camper_water/") + id + "/config";

    doc["name"] = name;
    doc["unique_id"] = uniqueId;
    doc["state_topic"] = stateTopic;
    doc["command_topic"] = cmdTopic;
    doc["payload_on"] = "ON";
    doc["payload_off"] = "OFF";
    doc["icon"] = icon;

    addDeviceBlock(doc);

    char buffer[512];
    size_t len = serializeJson(doc, buffer, sizeof(buffer));
    if (len == 0 || len >= sizeof(buffer)) {
        Serial.printf("[MQTT] Discovery payload for '%s' too large\n", id);
        return;
    }
    mqtt.publish(configTopic.c_str(), buffer, true);
}
