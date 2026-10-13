import os
import sys

Import("env")

# Cek apakah file .env ada
env_file_path = os.path.join(env.get("PROJECT_DIR"), ".env")

if not os.path.exists(env_file_path):
    print("\n[WARN] File .env tidak ditemukan! Menggunakan .env.example sebagai fallback.")
    env_file_path = os.path.join(env.get("PROJECT_DIR"), ".env.example")

defines = []

if os.path.exists(env_file_path):
    print(f"[ENV] Memuat konfigurasi dari: {env_file_path}")
    with open(env_file_path, "r") as f:
        for line in f:
            line = line.strip()
            # Abaikan komentar dan baris kosong
            if not line or line.startswith("#") or "=" not in line:
                continue
            key, val = line.split("=", 1)
            key = key.strip()
            val = val.strip()

            # Escape quotes untuk string C/C++ macro
            # Angka/float diteruskan langsung, string dibungkus tanda kutip
            if key in ["MQTT_BROKER_PORT", "TANK_TOTAL_DEPTH_CM", "TANK_SENSOR_OFFSET_MIN_CM", 
                        "TANK_THRESHOLD_LOW_PERCENT", "TANK_THRESHOLD_HIGH_PERCENT", "TELEMETRY_INTERVAL_MS"]:
                defines.append((f"ENV_{key}", val))
            else:
                defines.append((f"ENV_{key}", f'\\"{val}\\"'))

env.Append(CPPDEFINES=defines)
