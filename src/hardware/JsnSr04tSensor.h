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
        delayMicroseconds(5);

        // 2. Kirim pulsa trigger standar 20uS (TTL pulse > 10uS)
        digitalWrite(_triggerPin, HIGH);
        delayMicroseconds(20);
        digitalWrite(_triggerPin, LOW);

        // 3. Baca pulsa pantulan Echo
        unsigned long duration = pulseIn(_echoPin, HIGH, 35000);

        // Jika pulsa valid (antara 15 cm = 860 us s/d 400 cm = 23000 us)
        if (duration >= 860 && duration <= 25000) {
            float cm = duration / 57.5f; // Formula resmi datasheet JSN-SR04M-2
            Serial.printf("[SENSOR SUKSES] Pulsa: %lu us -> Jarak: %.1f cm\n", duration, cm);
            _lastValidDistanceCm = cm;
            return cm;
        }

        // Jika tidak ada pantulan pulsa
        Serial.printf("[SENSOR TIMEOUT] Pulsa: %lu us | D1 Pin: %s\n", 
                      duration, 
                      digitalRead(_echoPin) == HIGH ? "HIGH" : "LOW");
        return -1.0f;
    }
};
