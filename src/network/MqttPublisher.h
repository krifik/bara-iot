#pragma once

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "domain/WaterLevel.h"

class MqttPublisher {
private:
    WiFiClient _wifiClient;
    PubSubClient _mqttClient;
    const char* _brokerHost;
    uint16_t    _brokerPort;
    const char* _clientId;
    const char* _username;
    const char* _password;
    const char* _topicTelemetry;
    const char* _topicState;
    const char* _topicStatus;
    uint32_t    _lastReconnectAttempt;

public:
    MqttPublisher(const char* host, uint16_t port, const char* clientId,
                  const char* username, const char* password,
                  const char* topicTelemetry, const char* topicState, const char* topicStatus)
        : _mqttClient(_wifiClient),
          _brokerHost(host),
          _brokerPort(port),
          _clientId(clientId),
          _username(username),
          _password(password),
          _topicTelemetry(topicTelemetry),
          _topicState(topicState),
          _topicStatus(topicStatus),
          _lastReconnectAttempt(0) {}

    void begin() {
        _mqttClient.setServer(_brokerHost, _brokerPort);
        _mqttClient.setBufferSize(512);
    }

    void loop() {
        if (!_mqttClient.connected()) {
            uint32_t now = millis();
            if (now - _lastReconnectAttempt > 5000) {
                _lastReconnectAttempt = now;
                reconnect();
            }
        } else {
            _mqttClient.loop();
        }
    }

    bool isConnected() {
        return _mqttClient.connected();
    }

    bool reconnect() {
        if (WiFi.status() != WL_CONNECTED) return false;

        Serial.printf("[MQTT] Menghubungkan ke broker %s:%d...\n", _brokerHost, _brokerPort);

        // LWT (Last Will and Testament) payload saat node mati mendadak
        bool success = false;
        if (strlen(_username) > 0) {
            success = _mqttClient.connect(_clientId, _username, _password, _topicStatus, 1, true, "offline");
        } else {
            success = _mqttClient.connect(_clientId, _topicStatus, 1, true, "offline");
        }

        if (success) {
            Serial.println("[MQTT] Terhubung ke broker!");
            // Publish status online (retained message)
            _mqttClient.publish(_topicStatus, "online", true);
        } else {
            Serial.printf("[MQTT] Gagal terhubung, rc=%d\n", _mqttClient.state());
        }

        return success;
    }

    /**
     * @brief Publish telemetry JSON dan state string agar mudah di-consume service lain
     */
    bool publishTelemetry(const WaterLevelData& data, int8_t rssi, const String& ip) {
        if (!_mqttClient.connected()) return false;

        // 1. Publish State String sederhana (e.g. "LOW", "FULL", "NORMAL")
        // Sangat mudah di-subscribe oleh automation rule (misal Home Assistant / Node-RED)
        const char* stateStr = waterLevelStateToString(data.state);
        _mqttClient.publish(_topicState, stateStr, true);

        // 2. Publish Telemetry Lengkap dalam format JSON
        JsonDocument doc;
        doc["valid"]           = data.isValid;
        doc["state"]           = stateStr;
        doc["water_depth_cm"]  = serialized(String(data.waterDepthCm, 1));
        doc["percentage"]      = serialized(String(data.percentage, 1));
        doc["raw_distance_cm"] = serialized(String(data.rawDistanceCm, 1));
        doc["rssi"]            = rssi;
        doc["ip"]              = ip;
        doc["uptime_ms"]       = data.timestampMs;

        // Helper command hint untuk service lain:
        // Service pemutus air bisa langsung membaca field "pump_action"
        if (data.state == WaterLevelState::LOW_LEVEL || data.state == WaterLevelState::EMPTY) {
            doc["pump_action"] = "START";
        } else if (data.state == WaterLevelState::FULL || data.state == WaterLevelState::OVERFLOW) {
            doc["pump_action"] = "STOP";
        } else {
            doc["pump_action"] = "IDLE";
        }

        char buffer[512];
        size_t n = serializeJson(doc, buffer);

        return _mqttClient.publish(_topicTelemetry, buffer, n);
    }
};
