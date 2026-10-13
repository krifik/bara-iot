#include <Arduino.h>
#include "AppConfig.h"
#include "hardware/JsnSr04tSensor.h"
#include "services/WaterLevelService.h"
#include "network/NetworkManager.h"
#include "network/MqttPublisher.h"

// ==========================================
// Inisialisasi Objek & Dependency Injection
// ==========================================
static JsnSr04tSensor sensor(
    Config::Pins::TRIGGER_PIN,
    Config::Pins::ECHO_PIN,
    Config::Tank::MEDIAN_SAMPLE_COUNT
);

static WaterLevelService waterService(
    sensor,
    Config::Tank::TOTAL_DEPTH_CM,
    Config::Tank::SENSOR_OFFSET_MIN_CM,
    Config::Tank::THRESHOLD_LOW_PERCENT,
    Config::Tank::THRESHOLD_HIGH_PERCENT
);

static NetworkManager network(
    Config::Network::WIFI_SSID,
    Config::Network::WIFI_PASSWORD,
    Config::Network::RECONNECT_INTERVAL_MS
);

static MqttPublisher mqtt(
    Config::MQTT::BROKER_HOST,
    Config::MQTT::BROKER_PORT,
    Config::MQTT::CLIENT_ID,
    Config::MQTT::USERNAME,
    Config::MQTT::PASSWORD,
    Config::MQTT::TOPIC_TELEMETRY,
    Config::MQTT::TOPIC_STATE,
    Config::MQTT::TOPIC_STATUS
);

static uint32_t lastTelemetryTime = 0;

void setup() {
    Serial.begin(Config::Pins::STATUS_LED); // 115200 baud
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("==================================================");
    Serial.println("   BARA-IOT: WATER LEVEL AUTOMATION NODE (ESP8266) ");
    Serial.println("==================================================");

    pinMode(Config::Pins::STATUS_LED, OUTPUT);
    digitalWrite(Config::Pins::STATUS_LED, HIGH); // LED Builtin OFF

    // Inisialisasi Service & Hardware
    waterService.begin();
    network.begin();
    mqtt.begin();

    Serial.println("[Init] Sistem berhasil diinisialisasi.");
}

void loop() {
    // 1. Maintain Network & MQTT connection loops
    network.loop();
    mqtt.loop();

    // 2. Scheduled Telemetry & Measurement
    uint32_t currentMillis = millis();
    if (currentMillis - lastTelemetryTime >= Config::Tank::TELEMETRY_INTERVAL_MS) {
        lastTelemetryTime = currentMillis;

        // Indikasi LED kedip saat mengukur
        digitalWrite(Config::Pins::STATUS_LED, LOW);

        // Pengukuran
        WaterLevelData data = waterService.measure();

        // Print Diagnostic ke Serial Monitor
        if (data.isValid) {
            Serial.printf("[Sensor] Dist: %.1f cm | Depth: %.1f cm | Level: %.1f%% | State: %s\n",
                          data.rawDistanceCm,
                          data.waterDepthCm,
                          data.percentage,
                          waterLevelStateToString(data.state));
        } else {
            Serial.println("[Sensor] Gagal membaca sensor (Timeout / Out of range)");
        }

        // Publish ke MQTT jika terhubung
        if (network.isConnected()) {
            bool pubOk = mqtt.publishTelemetry(data, network.getRssi(), network.getIpAddress());
            if (pubOk) {
                Serial.println("[MQTT] Telemetry berhasil dipublish.");
            }
        }

        digitalWrite(Config::Pins::STATUS_LED, HIGH);
    }
}
