/*
 * Camper Waterlevel - ESP32 fresh water level sensor
 * ==================================================
 * Hardware: ESP32-C3 / ESP32 + Votronic analog tank sender
 *
 * Pin assignment:
 *   GPIO0 -> analog input (voltage measurement)
 */

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

#include "config.h"
#include "wifi_manager.h"
#include "sensor.h"
#include "mqtt_ha.h"
#include "webserver.h"

AppConfig config;
SensorData sensorData;

static unsigned long lastStatusPrint = 0;

void setupOTA() {
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(DEFAULT_OTA_PASSWORD);

    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        Serial.println("[OTA] Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\n[OTA] Update complete!");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        unsigned int pct = (total > 0) ? (progress * 100U / total) : 0;
        Serial.printf("[OTA] Progress: %u%%\r", pct);
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[OTA] Error[%u]\n", error);
    });

    ArduinoOTA.begin();
    Serial.println("[OTA] Ready");
}

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(50);  // Do not block the loop while waiting for a line end
    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("  Camper Waterlevel v1.0 (ESP32-C3)");
    Serial.println("========================================");
    Serial.println();

    memset(&sensorData, 0, sizeof(SensorData));

    // Phase 1: load config & mount LittleFS
    webserverInit();

    // Phase 2: initialize sensor/ADC
    sensorInit();

    // Phase 3: WiFi (blocking connect or AP fallback)
    wifiManagerInit();

    // Phase 4: start web server
    webserverStart();

    // Phase 5: OTA & MQTT
    setupOTA();
    mqttInit();

    Serial.printf("[MAIN] Setup done! Heap: %u\n", (unsigned)ESP.getFreeHeap());
}

void loop() {
    wifiManagerLoop();
    ArduinoOTA.handle();

    sensorLoop();
    mqttLoop();
    webserverLoop();

    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd == "read") {
            Serial.printf("[READ] Volt: %.3fV | Liter: %.1fL | Percent: %.0f%%\n",
                sensorData.avg_voltage, sensorData.liters, sensorData.percent);
        } else if (cmd == "filter") {
            config.filter_active = !config.filter_active;
            saveConfig();
            Serial.printf("[CMD] Filter: %s\n", config.filter_active ? "ON" : "OFF");
        } else if (cmd == "help") {
            Serial.println("[CMD] Commands: read, filter, help");
        }
    }

    if (millis() - lastStatusPrint > 10000) {
        lastStatusPrint = millis();
        Serial.printf("[STATUS] Volt:%.3fV Liter:%.1fL (%.0f%%) WiFi:%s MQTT:%s Heap:%u\n",
            sensorData.avg_voltage, sensorData.liters, sensorData.percent,
            getWiFiStateString().c_str(),
            mqttIsConnected() ? "OK" : "---",
            (unsigned)ESP.getFreeHeap());
    }

    delay(5);
    yield();
}
