#pragma once

#include "IWaterLevelSensor.h"
#include <Arduino.h>

/**
 * @brief Driver JSN-SR04M-2 Standard Pulse (Tanpa Memaksa D2 OUTPUT)
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
        // Biarkan Echo pin murni sebagai INPUT (jangan di-OUTPUT kan)
        pinMode(_echoPin, INPUT);
        
        // 1. Bersihkan trigger pin
        pinMode(_triggerPin, OUTPUT);
        digitalWrite(_triggerPin, LOW);
        delayMicroseconds(5);

        int echoBefore = digitalRead(_echoPin);

        // 2. Kirim pulsa trigger 20uS
        digitalWrite(_triggerPin, HIGH);
        delayMicroseconds(20);
        digitalWrite(_triggerPin, LOW);

        // 3. Baca pulsa menggunakan pulseIn standar
        // Jika echoBefore LOW: tunggu HIGH
        // Jika echoBefore HIGH: tunggu LOW
        unsigned long duration = 0;
        if (echoBefore == LOW) {
            duration = pulseIn(_echoPin, HIGH, 50000);
        } else {
            duration = pulseIn(_echoPin, LOW, 50000);
        }

        int echoAfter = digitalRead(_echoPin);
        Serial.printf("[ECHO] Sblm: %s | Pulsa: %lu us | Ssdh: %s\n",
                      echoBefore == HIGH ? "HIGH" : "LOW",
                      duration,
                      echoAfter == HIGH ? "HIGH" : "LOW");

        if (duration >= 200 && duration <= 35000) {
            float cm = duration / 57.5f;
            Serial.printf("[SENSOR SUKSES] Pulsa: %lu us -> %.1f cm\n", duration, cm);
            _lastValidDistanceCm = cm;
            return cm;
        }

        delay(80);
        return _lastValidDistanceCm;
    }
};
