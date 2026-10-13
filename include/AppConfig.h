#pragma once

#include <Arduino.h>

/**
 * @file AppConfig.h
 * @brief Application Configuration & Pin Definitions for NodeMCU ESP8266
 * Nilai default diambil secara otomatis dari file .env via build flags PlatformIO.
 */

namespace Config {

    // ==========================================
    // Pin Configuration (NodeMCU v3 CH340)
    // ==========================================
    namespace Pins {
        constexpr uint8_t TRIGGER_PIN   = D1; // GPIO5  -> Sensor RX (Kuning / Trigger)
        constexpr uint8_t ECHO_PIN      = D2; // GPIO4  -> Sensor TX (Biru / Echo)
        constexpr uint8_t STATUS_LED    = LED_BUILTIN; // Active LOW on NodeMCU
    }

    // ==========================================
    // Network & Wi-Fi Configuration
    // ==========================================
    namespace Network {
        #ifdef ENV_WIFI_SSID
            constexpr const char* WIFI_SSID     = ENV_WIFI_SSID;
        #else
            constexpr const char* WIFI_SSID     = "YOUR_WIFI_SSID";
        #endif

        #ifdef ENV_WIFI_PASSWORD
            constexpr const char* WIFI_PASSWORD = ENV_WIFI_PASSWORD;
        #else
            constexpr const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
        #endif

        constexpr uint32_t RECONNECT_INTERVAL_MS = 5000;
    }

    // ==========================================
    // MQTT Configuration
    // ==========================================
    namespace MQTT {
        #ifdef ENV_MQTT_BROKER_HOST
            constexpr const char* BROKER_HOST   = ENV_MQTT_BROKER_HOST;
        #else
            constexpr const char* BROKER_HOST   = "192.168.1.100";
        #endif

        #ifdef ENV_MQTT_BROKER_PORT
            constexpr uint16_t    BROKER_PORT   = ENV_MQTT_BROKER_PORT;
        #else
            constexpr uint16_t    BROKER_PORT   = 1883;
        #endif

        #ifdef ENV_MQTT_CLIENT_ID
            constexpr const char* CLIENT_ID     = ENV_MQTT_CLIENT_ID;
        #else
            constexpr const char* CLIENT_ID     = "bara-iot-water-node";
        #endif

        #ifdef ENV_MQTT_USERNAME
            constexpr const char* USERNAME      = ENV_MQTT_USERNAME;
        #else
            constexpr const char* USERNAME      = "";
        #endif

        #ifdef ENV_MQTT_PASSWORD
            constexpr const char* PASSWORD      = ENV_MQTT_PASSWORD;
        #else
            constexpr const char* PASSWORD      = "";
        #endif

        // MQTT Topics
        #ifdef ENV_MQTT_TOPIC_TELEMETRY
            constexpr const char* TOPIC_TELEMETRY   = ENV_MQTT_TOPIC_TELEMETRY;
        #else
            constexpr const char* TOPIC_TELEMETRY   = "home/water/telemetry";
        #endif

        #ifdef ENV_MQTT_TOPIC_STATE
            constexpr const char* TOPIC_STATE       = ENV_MQTT_TOPIC_STATE;
        #else
            constexpr const char* TOPIC_STATE       = "home/water/state";
        #endif

        #ifdef ENV_MQTT_TOPIC_STATUS
            constexpr const char* TOPIC_STATUS      = ENV_MQTT_TOPIC_STATUS;
        #else
            constexpr const char* TOPIC_STATUS      = "home/water/status";
        #endif

        constexpr uint16_t BUFFER_SIZE      = 512;
    }

    // ==========================================
    // Water Tank Physical Geometry & Calibration
    // ==========================================
    namespace Tank {
        #ifdef ENV_TANK_TOTAL_DEPTH_CM
            constexpr float TOTAL_DEPTH_CM          = ENV_TANK_TOTAL_DEPTH_CM;
        #else
            constexpr float TOTAL_DEPTH_CM          = 120.0f;
        #endif

        #ifdef ENV_TANK_SENSOR_OFFSET_MIN_CM
            constexpr float SENSOR_OFFSET_MIN_CM    = ENV_TANK_SENSOR_OFFSET_MIN_CM;
        #else
            constexpr float SENSOR_OFFSET_MIN_CM    = 20.0f;
        #endif

        #ifdef ENV_TANK_THRESHOLD_LOW_PERCENT
            constexpr float THRESHOLD_LOW_PERCENT   = ENV_TANK_THRESHOLD_LOW_PERCENT;
        #else
            constexpr float THRESHOLD_LOW_PERCENT   = 20.0f;
        #endif

        #ifdef ENV_TANK_THRESHOLD_HIGH_PERCENT
            constexpr float THRESHOLD_HIGH_PERCENT  = ENV_TANK_THRESHOLD_HIGH_PERCENT;
        #else
            constexpr float THRESHOLD_HIGH_PERCENT  = 90.0f;
        #endif

        #ifdef ENV_TELEMETRY_INTERVAL_MS
            constexpr uint32_t TELEMETRY_INTERVAL_MS = ENV_TELEMETRY_INTERVAL_MS;
        #else
            constexpr uint32_t TELEMETRY_INTERVAL_MS = 3000;
        #endif

        constexpr uint8_t  MEDIAN_SAMPLE_COUNT   = 5;
    }

}
