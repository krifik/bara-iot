#pragma once

#include <Arduino.h>

/**
 * @brief Water level state enumeration
 */
enum class WaterLevelState : uint8_t {
    EMPTY,       // Level < 10%
    LOW_LEVEL,   // Level <= THRESHOLD_LOW (Trig Pompa ON)
    NORMAL,      // Di antara Low dan High
    FULL,        // Level >= THRESHOLD_HIGH (Trig Pompa OFF)
    OVERFLOW,    // Level meluap melebihi batas aman
    SENSOR_ERROR // Sensor timeout / tidak terbaca
};

inline const char* waterLevelStateToString(WaterLevelState state) {
    switch (state) {
        case WaterLevelState::EMPTY:        return "EMPTY";
        case WaterLevelState::LOW_LEVEL:    return "LOW";
        case WaterLevelState::NORMAL:       return "NORMAL";
        case WaterLevelState::FULL:         return "FULL";
        case WaterLevelState::OVERFLOW:     return "OVERFLOW";
        case WaterLevelState::SENSOR_ERROR: return "SENSOR_ERROR";
        default:                            return "UNKNOWN";
    }
}

/**
 * @brief Domain Entity: Representasi data ketinggian air
 */
struct WaterLevelData {
    float rawDistanceCm;    // Jarak terbaca sensor (cm)
    float waterDepthCm;     // Ketinggian air dari dasar (cm)
    float percentage;       // Persentase kapasitas (0 - 100%)
    WaterLevelState state;  // Enum status tangki
    bool isValid;           // Validitas pengukuran
    uint32_t timestampMs;   // Uptime snapshot (ms)
};
