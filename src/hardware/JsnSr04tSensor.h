#pragma once

#include "IWaterLevelSensor.h"
#include <Arduino.h>

/**
 * @brief Driver JSN-SR04M-2 dengan Auto-Detect Pin Mapping (D1/D2 Bidirectional)
 * Otomatis mendeteksi polaritas dan pemetaan pin fisik secara real-time.
 */
class JsnSr04tSensor : public IWaterLevelSensor {
private:
    uint8_t _pinA; // D2
    uint8_t _pinB; // D1
    float _lastValidDistanceCm;

    float measurePair(uint8_t trigPin, uint8_t echoPin) {
        pinMode(echoPin, INPUT);
        pinMode(trigPin, OUTPUT);
        digitalWrite(trigPin, LOW);
        delayMicroseconds(5);

        int idleState = digitalRead(echoPin);

        // Kirim pulsa trigger 20uS
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(20);
        digitalWrite(trigPin, LOW);

        unsigned long duration = 0;
        if (idleState == LOW) {
            duration = pulseIn(echoPin, HIGH, 35000);
        } else {
            duration = pulseIn(echoPin, LOW, 35000);
        }

        // Jika duration valid (15 cm = 860us s/d 450 cm = 26000us)
        if (duration >= 860 && duration <= 30000) {
            return duration / 57.5f;
        }

        return -1.0f;
    }

public:
    JsnSr04tSensor(uint8_t pinA, uint8_t pinB)
        : _pinA(pinA), 
          _pinB(pinB), 
          _lastValidDistanceCm(-1.0f)
    {
    }

    void begin() override {
        pinMode(_pinA, OUTPUT);
        digitalWrite(_pinA, LOW);
        pinMode(_pinB, INPUT);
        _lastValidDistanceCm = -1.0f;
    }

    float readDistanceCm() override {
        // Coba Kombinasi 1: Trig = D2, Echo = D1
        float cm = measurePair(_pinA, _pinB);
        if (cm > 0.0f) {
            Serial.printf("[SENSOR SUKSES - MAP A] Jarak: %.1f cm (Trig: D2, Echo: D1)\n", cm);
            _lastValidDistanceCm = cm;
            return cm;
        }

        delay(40);

        // Coba Kombinasi 2: Trig = D1, Echo = D2 (Pin Terbalik)
        cm = measurePair(_pinB, _pinA);
        if (cm > 0.0f) {
            Serial.printf("[SENSOR SUKSES - MAP B] Jarak: %.1f cm (Trig: D1, Echo: D2)\n", cm);
            _lastValidDistanceCm = cm;
            return cm;
        }

        // Diagnostik status logika pin
        pinMode(_pinA, INPUT);
        pinMode(_pinB, INPUT);
        Serial.printf("[SENSOR TIMEOUT] D2: %s | D1: %s\n", 
                      digitalRead(_pinA) == HIGH ? "HIGH" : "LOW",
                      digitalRead(_pinB) == HIGH ? "HIGH" : "LOW");
        return -1.0f;
    }
};
