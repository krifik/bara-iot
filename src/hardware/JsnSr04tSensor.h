#pragma once

#include "IWaterLevelSensor.h"
#include <Arduino.h>

/**
 * @brief Driver JSN-SR04M-2 Standard TTL Pulse dengan Noise Spike Filter
 * Menghilangkan crosstalk kapasitif 4-5uS pada kabel jumper agar menangkap pantulan akustik asli.
 */
class JsnSr04tSensor : public IWaterLevelSensor {
private:
    uint8_t _triggerPin; // D2 -> Sensor RX (Kuning / Trigger)
    uint8_t _echoPin;    // D1 -> Sensor TX (Biru / Echo)
    float _lastValidDistanceCm;

public:
    JsnSr04tSensor(uint8_t triggerPin, uint8_t echoPin)
        : _triggerPin(triggerPin), 
          _echoPin(echoPin), 
          _lastValidDistanceCm(-1.0f)
    {
    }

    void begin() override {
        pinMode(_triggerPin, OUTPUT);
        digitalWrite(_triggerPin, LOW);
        pinMode(_echoPin, INPUT);
    }

    float readDistanceCm() override {
        pinMode(_echoPin, INPUT);
        pinMode(_triggerPin, OUTPUT);

        for (int attempt = 0; attempt < 3; attempt++) {
            // 1. Bersihkan trigger pin
            digitalWrite(_triggerPin, LOW);
            delayMicroseconds(10);

            // Pastikan pin Echo berada di kondisi LOW sebelum trigger
            unsigned long tWait = micros();
            while (digitalRead(_echoPin) == HIGH && (micros() - tWait < 2000)) {
                delayMicroseconds(10);
            }

            // 2. Kirim pulsa trigger TTL 20uS
            digitalWrite(_triggerPin, HIGH);
            delayMicroseconds(20);
            digitalWrite(_triggerPin, LOW);

            // 3. Lewati crosstalk listrik (4-10uS spike) sebelum transduser 40kHz selesai menembak
            delayMicroseconds(120);

            // 4. Baca pulsa pantulan ultrasonik sebenarnya (durasi HIGH)
            unsigned long duration = pulseIn(_echoPin, HIGH, 35000);

            // Durasi valid: 15 cm (860 us) s/d 450 cm (26000 us)
            if (duration >= 860 && duration <= 30000) {
                float cm = duration / 57.5f;
                Serial.printf("[SENSOR SUKSES] Pulsa: %lu us -> Jarak: %.1f cm\n", duration, cm);
                _lastValidDistanceCm = cm;
                return cm;
            }

            // Jika hanya menangkap spike listrik sesaat
            if (duration > 0 && duration < 860) {
                Serial.printf("[SENSOR NOISE FILTERED] Mengabaikan spike listrik: %lu us\n", duration);
            }

            delay(60); // Jeda redaman antar pengujian
        }

        Serial.printf("[SENSOR TIMEOUT] Tidak ada pantulan objek valid | D1: %s\n", 
                      digitalRead(_echoPin) == HIGH ? "HIGH" : "LOW");
        return -1.0f;
    }
};
