#pragma once

#include <Arduino.h>

/**
 * @file AppConfig.h
 * @brief Application Configuration & Pin Definitions for NodeMCU ESP8266
 */

namespace Config {

    // ==========================================
    // Pin Configuration (NodeMCU v3 CH340)
    // Note: D1 = GPIO5, D2 = GPIO4 (Safe for I/O & boot)
    // ==========================================
    namespace Pins {
        constexpr uint8_t TRIGGER_PIN   = D1; // GPIO5 -> JSN-SR04T Trig
        constexpr uint8_t ECHO_PIN      = D2; // GPIO4 -> JSN-SR04T Echo
        constexpr uint8_t STATUS_LED    = LED_BUILTIN; // Active LOW on NodeMCU
    }

    // ==========================================
    // Network & Wi-Fi Configuration
    // ==========================================
    namespace Network {
        constexpr const char* WIFI_SSID     = "YOUR_WIFI_SSID";
        constexpr const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
        constexpr uint32_t RECONNECT_INTERVAL_MS = 5000;
    }

    // ==========================================
    // MQTT Configuration
    // ==========================================
    namespace MQTT {
        constexpr const char* BROKER_HOST   = "192.168.1.100"; // Ganti dengan IP Mosquitto/EMQX
        constexpr uint16_t    BROKER_PORT   = 1883;
        constexpr const char* CLIENT_ID     = "bara-iot-water-node";
        constexpr const char* USERNAME      = ""; // Kosongkan jika tanpa auth
        constexpr const char* PASSWORD      = "";

        // MQTT Topics
        // telemetry/home/water/telemetry -> payload JSON detail (level, volume, distance, state)
        // telemetry/home/water/state     -> payload string singkat ("FILL", "FULL", "NORMAL", "LOW")
        constexpr const char* TOPIC_TELEMETRY   = "home/water/telemetry";
        constexpr const char* TOPIC_STATE       = "home/water/state";
        constexpr const char* TOPIC_STATUS      = "home/water/status"; // LWT (online/offline)

        constexpr uint16_t BUFFER_SIZE      = 512;
    }

    // ==========================================
    // Water Tank Physical Geometry & Calibration
    // ==========================================
    namespace Tank {
        // Jarak sensor ke dasar tangki saat kosong (cm)
        constexpr float TOTAL_DEPTH_CM          = 120.0f;

        // Jarak blind zone / batas minimum sensor ke air saat penuh (JSN-SR04T min reading ~20-25cm)
        constexpr float SENSOR_OFFSET_MIN_CM    = 20.0f;

        // Ambang batas level air dalam persen (%)
        constexpr float THRESHOLD_LOW_PERCENT   = 20.0f; // Waktunya menyalakan pompa/air
        constexpr float THRESHOLD_HIGH_PERCENT  = 90.0f; // Waktunya mematikan pompa/air

        // Interval pembacaan sensor dan publish MQTT (ms)
        constexpr uint32_t TELEMETRY_INTERVAL_MS = 3000;
        constexpr uint8_t  MEDIAN_SAMPLE_COUNT   = 5;    // Filter median noise reduction
    }

}
