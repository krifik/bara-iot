#include <Arduino.h>
#include "AppConfig.h"
#include "hardware/JsnSr04tSensor.h"
#include "services/WaterLevelService.h"
#include "network/NetworkManager.h"
#include "network/MqttPublisher.h"

// ==========================================
// Inisialisasi Objek & Dependency Injection
// ==========================================
// SR04M-2 Mode UART: D1 -> RX Sensor, D2 -> TX Sensor
static JsnSr04tSensor sensor(
    Config::Pins::TRIGGER_PIN, // D5 (Sensor RX)
    Config::Pins::ECHO_PIN     // D6 (Sensor TX)
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
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("==================================================");
    Serial.println("   BARA-IOT: WATER LEVEL AUTOMATION NODE (ESP8266) ");
    Serial.println("==================================================");

    pinMode(Config::Pins::STATUS_LED, OUTPUT);
    digitalWrite(Config::Pins::STATUS_LED, HIGH);

    // Inisialisasi Service & Hardware
    waterService.begin();
    network.begin();
    mqtt.begin();

    Serial.println("[Init] Sistem berhasil diinisialisasi.");
    Serial.println("[Init] Melakukan pembacaan awal sensor...");
    
    // Langsung ukur sekali saat boot tanpa menunggu interval
    WaterLevelData initialData = waterService.measure();
    Serial.printf("[Sensor Boot] Dist: %.1f cm\n", initialData.rawDistanceCm);
}


void loop() {
    network.loop();
    mqtt.loop();

    uint32_t currentMillis = millis();
    if (currentMillis - lastTelemetryTime >= Config::Tank::TELEMETRY_INTERVAL_MS) {
        lastTelemetryTime = currentMillis;

        // Indikasi LED kedip
        digitalWrite(Config::Pins::STATUS_LED, LOW);

        // Pengukuran Level Air
        WaterLevelData data = waterService.measure();

        // Print Diagnostic ke Serial Monitor
        if (data.isValid) {
            Serial.printf("[Sensor] Dist: %.1f cm | Kedalaman Air: %.1f cm | Kapasitas: %.1f%% | Status: %s\n",
                          data.rawDistanceCm,
                          data.waterDepthCm,
                          data.percentage,
                          waterLevelStateToString(data.state));
        } else {
            Serial.printf("[Sensor] Mencari sinyal ultrasonik... (Raw: %.1f cm)\n", data.rawDistanceCm);
        }

        // Publish ke MQTT
        if (network.isConnected()) {
            bool pubOk = mqtt.publishTelemetry(data, network.getRssi(), network.getIpAddress());
            if (pubOk) {
                Serial.println("[MQTT] Telemetry berhasil dipublish ke Broker!");
            }
        }

        digitalWrite(Config::Pins::STATUS_LED, HIGH);
    }
}
