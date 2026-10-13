#pragma once

#include "domain/WaterLevel.h"
#include "hardware/IWaterLevelSensor.h"

/**
 * @brief Water Level Measurement & Calculation Service
 * Menghitung kedalaman air efektif, persentase, dan state mesin berdasarkan parameter geometri tangki
 */
class WaterLevelService {
private:
    IWaterLevelSensor& _sensor;
    float _totalDepthCm;
    float _sensorOffsetMinCm;
    float _lowThresholdPct;
    float _highThresholdPct;

public:
    WaterLevelService(IWaterLevelSensor& sensor,
                      float totalDepthCm,
                      float sensorOffsetMinCm,
                      float lowThresholdPct,
                      float highThresholdPct)
        : _sensor(sensor),
          _totalDepthCm(totalDepthCm),
          _sensorOffsetMinCm(sensorOffsetMinCm),
          _lowThresholdPct(lowThresholdPct),
          _highThresholdPct(highThresholdPct) {}

    void begin() {
        _sensor.begin();
    }

    /**
     * @brief Menjalankan proses pengukuran dan konversi ke entitas WaterLevelData
     */
    WaterLevelData measure() {
        WaterLevelData result;
        result.timestampMs = millis();
        result.rawDistanceCm = _sensor.readDistanceCm();

        if (result.rawDistanceCm < 0.0f) {
            result.isValid = false;
            result.waterDepthCm = 0.0f;
            result.percentage = 0.0f;
            result.state = WaterLevelState::SENSOR_ERROR;
            return result;
        }

        result.isValid = true;

        // Effective water depth = total depth - (raw distance - offset)
        // Jika sensor ditempatkan tepat di atas bibir tangki:
        // Saat tangki penuh: rawDistance ~= sensorOffsetMinCm
        // Saat tangki kosong: rawDistance ~= totalDepthCm + sensorOffsetMinCm
        float effectiveMaxDistance = _totalDepthCm + _sensorOffsetMinCm;
        
        float depth = effectiveMaxDistance - result.rawDistanceCm;
        if (depth < 0.0f) depth = 0.0f;
        if (depth > _totalDepthCm) depth = _totalDepthCm;
        result.waterDepthCm = depth;

        // Hitung persentase kapasitas tangki (0 - 100%)
        result.percentage = (result.waterDepthCm / _totalDepthCm) * 100.0f;
        if (result.percentage < 0.0f) result.percentage = 0.0f;
        if (result.percentage > 100.0f) result.percentage = 100.0f;

        // Evaluasi State Mesin
        if (result.rawDistanceCm < _sensorOffsetMinCm && result.percentage >= 98.0f) {
            result.state = WaterLevelState::OVERFLOW;
        } else if (result.percentage >= _highThresholdPct) {
            result.state = WaterLevelState::FULL;
        } else if (result.percentage <= _lowThresholdPct) {
            result.state = WaterLevelState::LOW_LEVEL;
        } else if (result.percentage <= 5.0f) {
            result.state = WaterLevelState::EMPTY;
        } else {
            result.state = WaterLevelState::NORMAL;
        }

        return result;
    }
};
