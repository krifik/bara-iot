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

        // Test 1: Standard Active-HIGH Trigger (Idle LOW -> Pulse HIGH 30us -> LOW)
        digitalWrite(trigPin, LOW);
        delayMicroseconds(50);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(30);
        digitalWrite(trigPin, LOW);

        unsigned long duration = pulseIn(echoPin, HIGH, 35000);
        if (duration == 0) duration = pulseIn(echoPin, LOW, 20000);

        if (duration >= 860 && duration <= 30000) {
            return duration / 57.5f;
        }

        // Test 2: Inverted Active-LOW Trigger (Idle HIGH -> Pulse LOW 30us -> HIGH)
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(50);
        digitalWrite(trigPin, LOW);
        delayMicroseconds(30);
        digitalWrite(trigPin, HIGH);

        duration = pulseIn(echoPin, LOW, 35000);
        if (duration == 0) duration = pulseIn(echoPin, HIGH, 20000);

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
