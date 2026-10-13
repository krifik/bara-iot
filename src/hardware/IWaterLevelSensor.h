#pragma once

/**
 * @brief Interface Sensor Level Air
 * Memungkinkan dependency inversion dan mock testing tanpa hardware langsung
 */
class IWaterLevelSensor {
public:
    virtual ~IWaterLevelSensor() = default;

    /**
     * @brief Inisialisasi pin dan hardware sensor
     */
    virtual void begin() = 0;

    /**
     * @brief Mengukur jarak sensor ke permukaan air (cm)
     * @return float Jarak dalam cm, atau -1.0f jika error/timeout
     */
    virtual float readDistanceCm() = 0;
};
