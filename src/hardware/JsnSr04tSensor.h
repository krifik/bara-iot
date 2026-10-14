#pragma once

#include "IWaterLevelSensor.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

/**
 * @brief Driver JSN-SR04M-2 / 200427 Dual Engine
 * Sesuai Datasheet Resmi PDF:
 * 1. UART Mode (Instruction 0x55 & Auto-stream 4-byte frame @ 9600 baud)
 * 2. TTL Pulse Trigger Mode (Pulsa > 10uS & Echo pulse width)
 */
class JsnSr04tSensor : public IWaterLevelSensor {
private:
    uint8_t _triggerPin; // D2 -> Sensor RX (Kuning)
    uint8_t _echoPin;    // D1 -> Sensor TX (Biru)
    SoftwareSerial _serial;
    float _lastValidDistanceCm;

public:
    JsnSr04tSensor(uint8_t triggerPin, uint8_t echoPin)
        : _triggerPin(triggerPin), 
          _echoPin(echoPin), 
          _serial(echoPin, triggerPin), // RX = D1, TX = D2
          _lastValidDistanceCm(-1.0f)
    {
    }

    void begin() override {
        _serial.begin(9600);
        _lastValidDistanceCm = -1.0f;
    }

    float readDistanceCm() override {
        // ==========================================================
        // METODE 1: UART STREAM / INSTRUCTION 0x55 (Datasheet Hal 1 & 3)
        // ==========================================================
        
        // 1. Bersihkan buffer serial
        while (_serial.available() > 0) {
            _serial.read();
        }

        // 2. Kirim instruksi trigger UART 0x55
        _serial.write(0x55);

        // 3. Tunggu respon frame 4-byte (0xFF + H + L + Sum) hingga 80ms
        unsigned long startWait = millis();
        while (millis() - startWait < 80) {
            if (_serial.available() >= 4) {
                if (_serial.peek() != 0xFF) {
                    _serial.read();
                    continue;
                }
                uint8_t header = _serial.read();
                uint8_t h = _serial.read();
                uint8_t l = _serial.read();
                uint8_t cs = _serial.read();
                uint8_t expectedCs = (header + h + l) & 0xFF;

                if (cs == expectedCs) {
                    uint16_t distMm = (h << 8) | l;
                    float cm = distMm / 10.0f;
                    if (cm >= 15.0f && cm <= 600.0f) {
                        Serial.println("==================================================");
                        Serial.printf("[SENSOR SUKSES - UART] Jarak: %.1f cm (%u mm)\n", cm, distMm);
                        Serial.println("==================================================");
                        _lastValidDistanceCm = cm;
                        return cm;
                    }
                }
            }
            delay(5);
        }

        // ==========================================================
        // METODE 2: TTL PULSE MODE (Datasheet Hal 1 & 2)
        // ==========================================================
        _serial.stopListening(); // Nonaktifkan interrupt agar pulseIn presisi

        pinMode(_echoPin, INPUT);
        pinMode(_triggerPin, OUTPUT);
        digitalWrite(_triggerPin, LOW);
        delayMicroseconds(10);

        int idleBefore = digitalRead(_echoPin);

        // Pulsa trigger TTL 20uS
        digitalWrite(_triggerPin, HIGH);
        delayMicroseconds(20);
        digitalWrite(_triggerPin, LOW);

        // Ukur durasi pulsa
        unsigned long duration = 0;
        if (idleBefore == HIGH) {
            duration = pulseIn(_echoPin, LOW, 40000);
        } else {
            duration = pulseIn(_echoPin, HIGH, 40000);
        }

        // Aktifkan kembali SoftwareSerial untuk siklus berikutnya
        _serial.listen();

        if (duration >= 860 && duration <= 32000) {
            float cm = duration / 57.5f;
            Serial.println("==================================================");
            Serial.printf("[SENSOR SUKSES - PULSE] Pulsa: %lu us -> Jarak: %.1f cm\n", duration, cm);
            Serial.println("==================================================");
            _lastValidDistanceCm = cm;
            return cm;
        }

        // Jika kedua metode belum mendapatkan pantulan
        Serial.printf("[SENSOR TIMEOUT] UART: No Data | Pulse: %lu us | D1: %s\n",
                      duration,
                      idleBefore == HIGH ? "HIGH" : "LOW");
        return -1.0f;
    }
};
