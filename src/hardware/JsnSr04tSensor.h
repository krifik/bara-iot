#pragma once

#include "IWaterLevelSensor.h"
#include <Arduino.h>

/**
 * @brief Implementasi Driver JSN-SR04T / HC-SR04 dengan Median Filter
 * Mengeliminasi ripple riak air dan outlier bouncing signal pada tangki
 */
class JsnSr04tSensor : public IWaterLevelSensor {
private:
    uint8_t _triggerPin;
    uint8_t _echoPin;
    uint8_t _sampleCount;
    uint32_t _timeoutUs;

    /**
     * @brief Mengambil 1x single pulse echo time (mikrodetik)
     */
    unsigned long measureSinglePulseUs() {
        digitalWrite(_triggerPin, LOW);
        delayMicroseconds(2);
        digitalWrite(_triggerPin, HIGH);
        delayMicroseconds(15); // JSN-SR04T membutuhkan pulse minimal 10-20us
        digitalWrite(_triggerPin, LOW);

        return pulseIn(_echoPin, HIGH, _timeoutUs);
    }

public:
    JsnSr04tSensor(uint8_t triggerPin, uint8_t echoPin, uint8_t sampleCount = 5, uint32_t maxDistanceCm = 450)
        : _triggerPin(triggerPin), _echoPin(echoPin), _sampleCount(sampleCount) {
        // Konversi jarak maksimum ke timeout (jarak bolak-balik: 29.1 us/cm * 2)
        _timeoutUs = static_cast<uint32_t>(maxDistanceCm * 58.2f);
    }

    void begin() override {
        pinMode(_triggerPin, OUTPUT);
        pinMode(_echoPin, INPUT);
        digitalWrite(_triggerPin, LOW);
    }

    float readDistanceCm() override {
        if (_sampleCount <= 1) {
            unsigned long duration = measureSinglePulseUs();
            if (duration == 0) return -1.0f;
            return static_cast<float>(duration) / 58.2f;
        }

        // Sampling multiple readings untuk median filter
        float samples[15];
        uint8_t actualCount = (_sampleCount > 15) ? 15 : _sampleCount;
        uint8_t validSamples = 0;

        for (uint8_t i = 0; i < actualCount; ++i) {
            unsigned long duration = measureSinglePulseUs();
            if (duration > 0) {
                float dist = static_cast<float>(duration) / 58.2f;
                // JSN-SR04T rentang valid 20cm - 450cm
                if (dist >= 18.0f && dist <= 450.0f) {
                    samples[validSamples++] = dist;
                }
            }
            delay(35); // Jeda antar pulse ultrasonic agar tidak terjadi interferensi pantulan
        }

        if (validSamples == 0) {
            return -1.0f; // Sensor timeout / out of range
        }

        // Urutkan (Bubble sort sederhana untuk array kecil)
        for (uint8_t i = 0; i < validSamples - 1; ++i) {
            for (uint8_t j = 0; j < validSamples - i - 1; ++j) {
                if (samples[j] > samples[j + 1]) {
                    float temp = samples[j];
                    samples[j] = samples[j + 1];
                    samples[j + 1] = temp;
                }
            }
        }

        // Ambil nilai tengah (Median)
        return samples[validSamples / 2];
    }
};
