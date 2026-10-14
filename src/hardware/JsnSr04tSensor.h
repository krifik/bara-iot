#pragma once

#include "IWaterLevelSensor.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

/**
 * @brief Driver JSN-SR04M-2 / 200427 Mode UART Resmi (Datasheet DFRobot)
 * Mengirim instruksi 0x55 pada 9600 baud dan membaca frame 4-byte (0xFF + H + L + Sum)
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
        // 1. Bersihkan buffer serial sebelumnya
        while (_serial.available() > 0) {
            _serial.read();
        }

        // 2. Kirim instruksi trigger UART 0x55 ke Sensor RX (D2)
        _serial.write(0x55);

        // 3. Tunggu data balasan hingga 250ms (waktu pantulan + proses transduser)
        unsigned long startWait = millis();
        while (millis() - startWait < 250) {
            if (_serial.available() >= 4) {
                // Geser buffer hingga menemukan start byte 0xFF
                if (_serial.peek() != 0xFF) {
                    _serial.read();
                    continue;
                }

                uint8_t header = _serial.read();
                uint8_t h = _serial.read();
                uint8_t l = _serial.read();
                uint8_t cs = _serial.read();
                uint8_t expectedCs = (uint8_t)(header + h + l);

                if (cs == expectedCs) {
                    uint16_t distMm = (h << 8) | l;
                    float cm = distMm / 10.0f;

                    // Range valid JSN-SR04M-2: 20 cm s/d 600 cm
                    if (cm >= 15.0f && cm <= 600.0f) {
                        Serial.println("==================================================");
                        Serial.printf("[SENSOR SUKSES] Jarak: %.1f cm (Raw: %u mm | CS: 0x%02X)\n", cm, distMm, cs);
                        Serial.println("==================================================");
                        _lastValidDistanceCm = cm;
                        return cm;
                    }
                } else {
                    Serial.printf("[SENSOR UART CS ERROR] Expected 0x%02X, got 0x%02X\n", expectedCs, cs);
                }
            }
            delay(5);
        }

        int avail = _serial.available();
        Serial.printf("[SENSOR TIMEOUT] Buffer UART: %d byte tersedia\n", avail);
        if (avail > 0) {
            Serial.print("[SENSOR RAW BYTES]: ");
            while (_serial.available() > 0) {
                Serial.printf("0x%02X ", _serial.read());
            }
            Serial.println();
        }

        return -1.0f;
    }
};
