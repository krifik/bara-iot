#pragma once

#include <ESP8266WiFi.h>

class NetworkManager {
private:
    const char* _ssid;
    const char* _password;
    uint32_t _reconnectIntervalMs;
    uint32_t _lastAttemptMs;

public:
    NetworkManager(const char* ssid, const char* password, uint32_t reconnectIntervalMs = 5000)
        : _ssid(ssid), _password(password), _reconnectIntervalMs(reconnectIntervalMs), _lastAttemptMs(0) {}

    void begin() {
        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);
        connect();
    }

    void connect() {
        if (WiFi.status() == WL_CONNECTED) return;

        Serial.printf("[WiFi] Menghubungkan ke SSID: %s\n", _ssid);
        WiFi.begin(_ssid, _password);
        _lastAttemptMs = millis();
    }

    void loop() {
        if (WiFi.status() != WL_CONNECTED) {
            uint32_t now = millis();
            if (now - _lastAttemptMs >= 10000) {
                _lastAttemptMs = now;
                Serial.printf("[WiFi] Status: %d. Menghubungkan ulang ke SSID: %s...\n", WiFi.status(), _ssid);
                WiFi.begin(_ssid, _password);
            }
        }
    }



    bool isConnected() const {
        return WiFi.status() == WL_CONNECTED;
    }

    String getIpAddress() const {
        return WiFi.localIP().toString();
    }

    int8_t getRssi() const {
        return WiFi.RSSI();
    }
};
