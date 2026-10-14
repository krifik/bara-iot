#pragma once

#include "IWaterLevelSensor.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

/**
 * @brief Driver JSN-SR04M-2 Dual-Engine:
 * 1. Serial UART Engine (Auto-stream 0xFF & Command 0x55) - Sesuai Datasheet Resmi Halaman 1 & 3
 * 2. TTL Pulse Trigger Engine (Eksitasi pulsa 100uS)
 */
class JsnSr04tSensor : public IWaterLevelSensor {
private:
    uint8_t _triggerPin; // D2 -> RX Sensor (Kuning)
    uint8_t _echoPin;    // D1 -> TX Sensor (Biru)
    SoftwareSerial _serial;
    float _lastValidDistanceCm;

public:
    JsnSr04tSensor(uint8_t triggerPin, uint8_t echoPin)
        : _triggerPin(triggerPin), 
          _echoPin(echoPin), 
          _serial(echoPin, triggerPin), // RX = D1 (Sensor TX), TX = D2 (Sensor RX)
          _lastValidDistanceCm(-1.0f)
    {
    }

    void begin() override {
        _serial.begin(9600);
        pinMode(_triggerPin, OUTPUT);
        digitalWrite(_triggerPin, LOW);
        pinMode(_echoPin, INPUT);
    }

    float readDistanceCm() override {
        // ========================================================
        // 1. UJI SERIAL UART AUTO-STREAM (Datasheet Hal 3)
        // Jika modul di mode UART otomatis, LED akan berkedip cepat
        // dan mengirimkan 4-byte frame: 0xFF + Data_H + Data_L + Checksum
        // ========================================================
        if (_serial.available() >= 4) {
            while (_serial.available() >= 4) {
                if (_serial.read() == 0xFF) {
                    uint8_t h = _serial.read();
                    uint8_t l = _serial.read();
                    uint8_t cs = _serial.read();
                    if (cs == (uint8_t)(0xFF + h + l)) {
                        uint16_t distMm = (h << 8) | l;
                        float cm = distMm / 10.0f;
                        if (cm >= 15.0f && cm <= 600.0f) {
                            Serial.println("==================================================");
                            Serial.printf("[SENSOR UART-AUTO] Jarak: %.1f cm (%u mm)\n", cm, distMm);
                            Serial.println("==================================================");
                            _lastValidDistanceCm = cm;
                            return cm;
                        }
                    }
                }
            }
        }

        // ========================================================
        // 2. UJI SERIAL COMMAND 0x55 (Datasheet Hal 1 Poin 2)
        // Kirim perintah trigger 0x55 ke RX sensor (D2)
        // ========================================================
        while (_serial.available() > 0) _serial.read(); // Bersihkan buffer
        _serial.write(0x55);
        delay(20);
        if (_serial.available() >= 4) {
            if (_serial.read() == 0xFF) {
                uint8_t h = _serial.read();
                uint8_t l = _serial.read();
                uint8_t cs = _serial.read();
                if (cs == (uint8_t)(0xFF + h + l)) {
                    uint16_t distMm = (h << 8) | l;
                    float cm = distMm / 10.0f;
                    if (cm >= 15.0f && cm <= 600.0f) {
                        Serial.println("==================================================");
                        Serial.printf("[SENSOR UART-CMD 0x55] Jarak: %.1f cm (%u mm)\n", cm, distMm);
                        Serial.println("==================================================");
                        _lastValidDistanceCm = cm;
                        return cm;
                    }
                }
            }
        }

        // ========================================================
        // 3. TTL PULSE MODE DENGAN PULSA EKSITASI 100uS
        // ========================================================
        pinMode(_echoPin, INPUT);
        int echoRaw = digitalRead(_echoPin);

        // Kirim pulsa trigger 100uS kuat ke D2
        pinMode(_triggerPin, OUTPUT);
        digitalWrite(_triggerPin, LOW);
        delayMicroseconds(10);
        digitalWrite(_triggerPin, HIGH);
        delayMicroseconds(100);
        digitalWrite(_triggerPin, LOW);

        unsigned long duration = 0;
        if (echoRaw == LOW) {
            duration = pulseIn(_echoPin, HIGH, 40000);
        } else {
            duration = pulseIn(_echoPin, LOW, 40000);
        }

        Serial.printf("[DIAG] D1(Echo): %s | Duration: %lu us | UART Avail: %d byte\n",
                      echoRaw == HIGH ? "HIGH" : "LOW",
                      duration,
                      _serial.available());

        if (duration >= 860 && duration <= 30000) {
            float cm = duration / 57.5f;
            Serial.println("==================================================");
            Serial.printf("[SENSOR PULSE SUKSES] Pulsa: %lu us -> Jarak: %.1f cm\n", duration, cm);
            Serial.println("==================================================");
            _lastValidDistanceCm = cm;
            return cm;
        }

        delay(60);
        return _lastValidDistanceCm;
    }
};
