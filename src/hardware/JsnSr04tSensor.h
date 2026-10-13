#pragma once

#include "IWaterLevelSensor.h"
#include <Arduino.h>

/**
 * @brief Driver JSN-SR04M-2 dengan Multi-Burst Sampling & Glitch Recovery
 */
class JsnSr04tSensor : public IWaterLevelSensor {
private:
    uint8_t _triggerPin;
    uint8_t _echoPin;
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

        // Ambil hingga 5 sampel burst cepat untuk mendapatkan pantulan suara nyata
        float validSamples[5];
        int sampleCount = 0;

        for (int attempt = 0; attempt < 5; attempt++) {
            // 1. Bersihkan trigger pin
            pinMode(_triggerPin, OUTPUT);
            digitalWrite(_triggerPin, LOW);
            delayMicroseconds(5);

            // Tunggu jika pin Echo belum LOW dari siklus sebelumnya (maks 2ms)
            unsigned long tWait = micros();
            while (digitalRead(_echoPin) == HIGH && (micros() - tWait < 2000)) {
                delayMicroseconds(10);
            }

            int echoBefore = digitalRead(_echoPin);

            // 2. Kirim pulsa trigger 20uS
            digitalWrite(_triggerPin, HIGH);
            delayMicroseconds(20);
            digitalWrite(_triggerPin, LOW);

            // 3. Baca pulsa Echo
            unsigned long duration = 0;
            if (echoBefore == LOW) {
                duration = pulseIn(_echoPin, HIGH, 40000);
            } else {
                duration = pulseIn(_echoPin, LOW, 40000);
            }

            // Jika pulsa valid (antara 15 cm = 860 us s/d 450 cm = 26000 us)
            if (duration >= 860 && duration <= 30000) {
                float cm = duration / 57.5f;
                validSamples[sampleCount++] = cm;
            }

            delay(60); // Jeda redaman antar pengujian (> 50ms)
        }

        // Jika berhasil mendapatkan minimal 1 sampel valid
        if (sampleCount > 0) {
            // Hitung rata-rata sampel yang berhasil ditangkap
            float sum = 0;
            for (int i = 0; i < sampleCount; i++) {
                sum += validSamples[i];
            }
            float avgCm = sum / sampleCount;

            Serial.println("==================================================");
            Serial.printf("[SENSOR SUKSES] Berhasil %d/5 Sampel -> Rata-rata Jarak: %.1f cm\n", sampleCount, avgCm);
            Serial.println("==================================================");

            _lastValidDistanceCm = avgCm;
            return avgCm;
        }

        // Jika semua 5 sampel gagal, tandai sensor kehilangan jejak
        Serial.println("[SENSOR RETRY] Tidak ada pantulan suara terdeteksi dalam 5x pengujian");
        _lastValidDistanceCm = -1.0f;
        return -1.0f;
    }
};
