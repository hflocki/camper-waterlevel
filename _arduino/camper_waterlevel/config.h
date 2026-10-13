#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================
// PIN DEFINITIONS & CONSTANTS
// ============================================================
#define ADC_PIN               0      // GPIO0 (ADC1_CH0) for the Votronic tank sender
#define ADC_SAMPLES           8      // Oversampling per measurement (reduces ADC noise)

#define DEFAULT_AP_SSID       "Camper-Waterlevel"
#define DEFAULT_AP_PASSWORD   "waterlevel_ap_PW"   // placeholder (min. 8 characters)
#define DEFAULT_OTA_PASSWORD  "waterlevel_ota_PW"  // placeholder (min. 8 characters)
#define DEFAULT_MQTT_SERVER   "192.168.1.10"        // example - set your broker in the web UI
#define DEFAULT_MQTT_PORT     1883
#define DEFAULT_MQTT_PREFIX   "camper/waterlevel"
#define DEFAULT_TANK_SIZE     100.0f  // Tank capacity in liters (changeable in the web UI)

#define SENSOR_UPDATE_MS      1000   // Read sensor every second
#define FILTER_WINDOW_SIZE    75     // Moving-average samples: 75 x 1 s = 75 s (ESPHome: 15 x 5 s)

// WiFi TX power in 0.25 dBm steps: 70 = 17.5 dBm (same as ESPHome output_power: 17.5dBm).
// Full power (19.5 dBm) tends to be unstable on ESP32-C3 Super Mini boards.
#define WIFI_TX_POWER         ((wifi_power_t)70)
#define MQTT_FAST_MS          1000   // Fast publish interval
#define MQTT_SLOW_MS          5000   // Normal publish interval
#define WS_UPDATE_MS          1000   // Web UI live update interval

#define CONFIG_FILE           "/config.json"
#define OTA_HOSTNAME          "camper-waterlevel"

#define MAX_CALIB_POINTS      15     // Max. number of calibration points

// ============================================================
// DYNAMIC CALIBRATION TABLE
// ============================================================
struct CalibPoint {
    float volt;
    float percent;   // Fill level in % (0-100) at this voltage; liters = percent * tank size
};

// ============================================================
// CONFIGURATION STRUCT
// ============================================================
struct AppConfig {
    char wifi_ssid[64];
    char wifi_pass[64];
    char ap_ssid[32];
    char ap_pass[32];

    char mqtt_server[64];
    uint16_t mqtt_port;
    char mqtt_user[32];
    char mqtt_pass[32];
    char mqtt_prefix[32];

    float tank_size;       // Tank capacity in liters
    bool filter_active;
    bool mqtt_fast;

    // Dynamic volt-to-liter table (sorted by voltage, ascending)
    CalibPoint calibTable[MAX_CALIB_POINTS];
    uint8_t calibCount;
};

// ============================================================
// SENSOR DATA STRUCT
// ============================================================
struct SensorData {
    float raw_voltage;     // Raw voltage from the ADC
    float avg_voltage;     // Filtered voltage
    float liters;          // Calculated content in liters
    float percent;         // Calculated content in % (0-100)
    bool sensor_ok;
    int rssi;
};

extern AppConfig config;
extern SensorData sensorData;

#endif // CONFIG_H
