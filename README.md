# BARA-IoT: Water Level Detection & Automation Node

Modul IoT pendeteksi ketinggian air (*water level monitoring*) berbasis **NodeMCU V3 ESP8266 (CH340)** dan sensor ultrasonik tahan air **JSN-SR04T**, dirancang untuk integrasi otomatisasi Smart Home (misal: otomatis mematikan/menyalakan pompa air) menggunakan protokol **MQTT**.

---

## 🏗️ Clean Architecture & Struktur Direktori

Kode dirancang dengan pemisahan dependensi (*Dependency Inversion*), memisahkan driver hardware, kalkulasi domain bisnis, dan layer network:

```text
bara-iot/
├── include/
│   └── AppConfig.h                  # Konfigurasi WiFi, Broker MQTT, Pin, & Geometri Tangki
├── src/
│   ├── domain/
│   │   └── WaterLevel.h             # Domain Entity (WaterLevelData, Enum State)
│   ├── hardware/
│   │   ├── IWaterLevelSensor.h      # Abstraksi Interface Sensor
│   │   └── JsnSr04tSensor.h         # Driver JSN-SR04T + Median Filter
│   ├── services/
│   │   └── WaterLevelService.h      # Logika kalkulasi kedalaman, persen, & state
│   ├── network/
│   │   ├── NetworkManager.h         # Wi-Fi auto-reconnect manager
│   │   └── MqttPublisher.h          # MQTT Publisher (Telemetry JSON & State String)
│   └── main.cpp                     # Composition Root (Dependency Injection & Loop)
├── platformio.ini                   # Konfigurasi PlatformIO & Dependencies
└── README.md
```

---

## 🔌 Skema Pin (NodeMCU v3 CH340 <-> JSN-SR04T)

| Pin JSN-SR04T | Pin NodeMCU V3 | GPIO | Keterangan |
| :--- | :--- | :--- | :--- |
| **5V / VCC** | **VIN / 5V** | - | JSN-SR04T butuh tegangan 5V stabil |
| **GND** | **GND** | - | Ground bersama |
| **Trig** | **D1** | GPIO5 | Sinyal Trigger Pulsa 15µs |
| **Echo** | **D2** | GPIO4 | Sinyal Pembacaan Pantulan Echo |

> **Catatan Hardware**: 
> Sensor **JSN-SR04T** memiliki blind-zone / jarak minimum pembacaan sekitar 20-25 cm. Pastikan posisi probe diletakkan minimal 20 cm di atas batas air tertinggi tangki.

---

## 📡 MQTT Payload & API Contract

Service eksternal (Home Assistant / Node-RED / Go / Python Service pemutus air) dapat meng-consume 2 topik utama:

### 1. Topik State Ringkas: `home/water/state`
Berisi payload string:
- `"LOW"`: Air di bawah ambang batas minimum (perlu dinyalakan)
- `"NORMAL"`: Air di level aman
- `"FULL"`: Air telah mencapai batas maksimum (segera matikan)
- `"OVERFLOW"`: Air melebihi batas aman (darurat mati)
- `"SENSOR_ERROR"`: Gangguan pembacaan sensor

### 2. Topik Telemetri Lengkap: `home/water/telemetry`
Payload format JSON:
```json
{
  "valid": true,
  "state": "LOW",
  "water_depth_cm": 18.5,
  "percentage": 15.4,
  "raw_distance_cm": 121.5,
  "pump_action": "START",
  "rssi": -65,
  "ip": "192.168.1.45",
  "uptime_ms": 120530
}
```
> **Tip Otomasi:** Service lain cukup membaca parameter `"pump_action"` (`"START"` / `"STOP"` / `"IDLE"`) atau nilai `"percentage"` untuk kontrol saklar/relay pompa air.

---

## 🚀 Cara Menjalankan & Upload

1. Buka folder ini di **VS Code** dengan ekstensi **PlatformIO IDE**.
2. Sesuaikan konfigurasi di `include/AppConfig.h`:
   - `WIFI_SSID` & `WIFI_PASSWORD`
   - `BROKER_HOST` & `BROKER_PORT`
   - `TOTAL_DEPTH_CM` (kedalaman tangki Anda)
   - `SENSOR_OFFSET_MIN_CM` (jarak sensor ke bibir air penuh)
3. Sambungkan NodeMCU V3 CH340 ke USB.
4. Klik **Build** (tanda centang) dan **Upload** (tanda panah) di PlatformIO.
