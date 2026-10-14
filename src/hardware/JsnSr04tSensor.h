#pragma once

#include "IWaterLevelSensor.h"
#include <Arduino.h>

/**
 * @brief Driver JSN-SR04M-2 / 200427 dengan Deterministic Edge Detection
 * Otomatis menangani Idle-HIGH maupun Idle-LOW, serta memfilter spike crosstalk 4uS
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
        _lastValidDistanceCm = -1.0f;
    }

    float readDistanceCm() override {
        pinMode(_echoPin, INPUT);
        pinMode(_triggerPin, OUTPUT);
        digitalWrite(_triggerPin, LOW);
        delayMicroseconds(10);

        int idle = digitalRead(_echoPin); // Deteksi idle state D1 (HIGH atau LOW)

        // 1. Kirim pulsa trigger 20uS
        digitalWrite(_triggerPin, HIGH);
        delayMicroseconds(20);
        digitalWrite(_triggerPin, LOW);

        // 2. Tunggu transisi edge pertama dari idle state (maksimal 45ms)
        unsigned long tWait = micros();
        while (digitalRead(_echoPin) == idle) {
            if (micros() - tWait > 45000) {
                Serial.printf("[SENSOR TIMEOUT] Tidak ada respons transisi D1 dalam 45ms (Idle: %s)\n", 
                              idle == HIGH ? "HIGH" : "LOW");
                return -1.0f;
            }
        }

        // 3. Pin telah berubah state! Mulai hitung durasi
        unsigned long tStart = micros();
        while (digitalRead(_echoPin) != idle) {
            if (micros() - tStart > 45000) {
                Serial.printf("[SENSOR TIMEOUT] Pulsa melebihi batas 45ms\n");
                return -1.0f;
            }
        }
        unsigned long duration = micros() - tStart;

        // 4. Jika pulsa pertama adalah spike crosstalk kapasitif (< 860us / < 15cm)
        if (duration < 860) {
            Serial.printf("[SENSOR SPIKE] Mengabaikan noise %lu us, menunggu pantulan utama...\n", duration);
            
            // Tunggu pulsa pantulan akustik sesungguhnya (hingga 45ms)
            tWait = micros();
            while (digitalRead(_echoPin) == idle) {
                if (micros() - tWait > 45000) {
                    Serial.println("[SENSOR TIMEOUT] Pantulan kedua tidak muncul");
                    return -1.0f;
                }
            }
            tStart = micros();
            while (digitalRead(_echoPin) != idle) {
                if (micros() - tStart > 45000) {
                    return -1.0f;
                }
            }
            duration = micros() - tStart;
        }

        // 5. Validasi durasi pantulan nyata (15 cm = 860us s/d 500 cm = 28750us)
        if (duration >= 860 && duration <= 32000) {
            float cm = duration / 57.5f; // Formula resmi datasheet JSN-SR04M-2
            Serial.println("==================================================");
            Serial.printf("[SENSOR SUKSES] Pulsa: %lu us -> Jarak: %.1f cm (Idle: %s)\n", 
                          duration, cm, idle == HIGH ? "HIGH" : "LOW");
            Serial.println("==================================================");
            _lastValidDistanceCm = cm;
            return cm;
        }

        Serial.printf("[SENSOR TIMEOUT] Pulsa akhir: %lu us | D1: %s\n", 
                      duration, digitalRead(_echoPin) == HIGH ? "HIGH" : "LOW");
        return -1.0f;
    }
};
