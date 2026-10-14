#pragma once

#include "IWaterLevelSensor.h"
#include <Arduino.h>

/**
 * @brief Driver JSN-SR04M-2 Standard TTL Pulse
 * PERINGATAN KERAS: Pin Echo (D1) adalah OUTPUT dari sensor.
 * Pin ini HARUS SELALU INPUT pada ESP8266 untuk mencegah short-circuit.
 */
class JsnSr04tSensor : public IWaterLevelSensor {
private:
    uint8_t _triggerPin; // D2 -> RX Sensor (Kuning / Trigger)
    uint8_t _echoPin;    // D1 -> TX Sensor (Biru / Echo)
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
        // Echo pin WAJIB selalu INPUT murni
        pinMode(_echoPin, INPUT);

        // 1. Bersihkan trigger pin
        pinMode(_triggerPin, OUTPUT);
        digitalWrite(_triggerPin, LOW);
        delayMicroseconds(10);

        // Deteksi status idle pin Echo (D1)
        int idleState = digitalRead(_echoPin);

        // 2. Kirim pulsa trigger bersih 30uS (>10uS TTL standar SR04M-2)
        digitalWrite(_triggerPin, HIGH);
        delayMicroseconds(30);
        digitalWrite(_triggerPin, LOW);

        // 3. Baca pulsa pantulan Echo:
        // Pada SR04M-2 yang idle HIGH, pulsa pantulan adalah transisi Active-LOW
        // Pada modul standard yang idle LOW, pulsa pantulan adalah Active-HIGH
        unsigned long duration = 0;
        if (idleState == HIGH) {
            duration = pulseIn(_echoPin, LOW, 45000);
        } else {
            duration = pulseIn(_echoPin, HIGH, 45000);
        }

        // Fallback jika polaritas berubah mendadak
        if (duration == 0) {
            duration = (idleState == HIGH) ? pulseIn(_echoPin, HIGH, 5000) : pulseIn(_echoPin, LOW, 5000);
        }

        // Jika pulsa valid (antara 15 cm = 860 us s/d 500 cm = 28750 us)
        if (duration >= 860 && duration <= 32000) {
            float cm = duration / 57.5f; // Formula resmi datasheet JSN-SR04M-2
            Serial.printf("[SENSOR SUKSES] Pulsa: %lu us -> Jarak: %.1f cm (D1 Idle: %s)\n", 
                          duration, cm, idleState == HIGH ? "HIGH" : "LOW");
            _lastValidDistanceCm = cm;
            return cm;
        }

        // Jika tidak ada pantulan pulsa
        Serial.printf("[SENSOR TIMEOUT] Pulsa: %lu us | D1 Idle: %s | D1 Now: %s\n", 
                      duration, 
                      idleState == HIGH ? "HIGH" : "LOW",
                      digitalRead(_echoPin) == HIGH ? "HIGH" : "LOW");
        return -1.0f;
    }
};
