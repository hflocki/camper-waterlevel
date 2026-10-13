#include "sensor.h"

static unsigned long lastSensorRead = 0;

// Moving-average filter buffer
static const int WINDOW_SIZE = FILTER_WINDOW_SIZE;
static float readings[WINDOW_SIZE];
static int readIndex = 0;
static bool bufferPrimed = false;

// Linear interpolation over config.calibTable (sorted by voltage); returns the fill level in %
static float interpolatePercent(float volt) {
    if (config.calibCount == 0) return 0.0f;

    // Clamp below the first and above the last point
    if (volt <= config.calibTable[0].volt) return config.calibTable[0].percent;
    if (volt >= config.calibTable[config.calibCount - 1].volt) return config.calibTable[config.calibCount - 1].percent;

    for (int i = 0; i < config.calibCount - 1; i++) {
        if (volt >= config.calibTable[i].volt && volt <= config.calibTable[i + 1].volt) {
            float vDiff = config.calibTable[i + 1].volt - config.calibTable[i].volt;
            if (vDiff <= 0.0001f) return config.calibTable[i].percent;  // Guard against division by zero
            float t = (volt - config.calibTable[i].volt) / vDiff;
            return config.calibTable[i].percent + t * (config.calibTable[i + 1].percent - config.calibTable[i].percent);
        }
    }
    return 0.0f;
}

// Reads the pin voltage in volts using the chip's factory ADC calibration
static float readVoltage() {
    uint32_t sumMv = 0;
    for (int i = 0; i < ADC_SAMPLES; i++) {
        sumMv += analogReadMilliVolts(ADC_PIN);
    }
    return (sumMv / (float)ADC_SAMPLES) / 1000.0f;
}

bool sensorInit() {
    Serial.println("[Sensor] Initializing analog input (GPIO0)...");

    analogReadResolution(12);                        // 12 bit (0-4095)
    analogSetPinAttenuation(ADC_PIN, ADC_11db);      // Widest input range (approx. 0-2.5 V on the ESP32-C3)

    for (int i = 0; i < WINDOW_SIZE; i++) {
        readings[i] = 0.0f;
    }
    bufferPrimed = false;

    sensorData.sensor_ok = true;
    return true;
}

void sensorLoop() {
    if (millis() - lastSensorRead < SENSOR_UPDATE_MS) return;
    lastSensorRead = millis();

    float voltage = readVoltage();
    sensorData.raw_voltage = voltage;

    // Prefill the filter with the first reading so the average does not start at 0 V
    if (!bufferPrimed) {
        for (int i = 0; i < WINDOW_SIZE; i++) readings[i] = voltage;
        bufferPrimed = true;
    }

    // Moving average (buffer is always updated, even if the filter is switched off)
    readings[readIndex] = voltage;
    readIndex = (readIndex + 1) % WINDOW_SIZE;

    float sum = 0.0f;
    for (int i = 0; i < WINDOW_SIZE; i++) sum += readings[i];

    float avgVoltage = config.filter_active ? (sum / WINDOW_SIZE) : voltage;
    sensorData.avg_voltage = avgVoltage;

    // Convert voltage to fill level in % using the table, then to liters using the tank size
    float percent = interpolatePercent(avgVoltage);
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    sensorData.percent = percent;
    sensorData.liters = percent / 100.0f * config.tank_size;
}
