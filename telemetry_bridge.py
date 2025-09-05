#!/usr/bin/env python3
import time
import math
import random

# Thresholds for fault detection
ALTITUDE_THRESHOLD = 10.0  # km deviation from nominal
GYRO_THRESHOLD = 0.5        # rad/s
MAG_THRESHOLD = 0.5         # arbitrary units

class TelemetryBridge:
    def __init__(self):
        self.time_sec = 0
        self.nominal_altitude = 360.0
        self.altitude = self.nominal_altitude
        self.gyro = [0.0, 0.0, 0.0]
        self.magnetometer = [0.0, 0.0, 0.0]

        print("Starting simple telemetry bridge (normal run)")

    # --- Fault injection (optional) ---
    # def inject_faults(self):
    #     if self.time_sec == 10:
    #         delta = random.uniform(-40.0, -30.0)
    #         self.altitude += delta
    #         print(f"!!!FAULT INJECTION: altitude changed by {delta:.2f} km!!!")
    #     if self.time_sec == 16:
    #         delta = random.uniform(15.0, 20.0)
    #         self.altitude += delta
    #         self.gyro[1] += random.uniform(-0.6, -0.5)
    #         print(f"!!!FAULT INJECTION: altitude changed by {delta:.2f} km, gyro[1] changed by {self.gyro[1]:.3f}!!!")

    def check_status(self):
        status = "NORMAL"
        if abs(self.altitude - self.nominal_altitude) > ALTITUDE_THRESHOLD:
            status = "FAIL"
        if any(abs(g) > GYRO_THRESHOLD for g in self.gyro):
            status = "FAIL"
        if any(abs(m) > MAG_THRESHOLD for m in self.magnetometer):
            status = "FAIL"
        return status

    def run(self):
        while True:
            # --- Normal gradual changes ---
            decay = 0.001 + 0.002 * math.sin(self.time_sec * 0.1)
            self.altitude -= decay
            self.gyro = [
                0.01 * math.sin(self.time_sec * 0.1),
                0.005 * math.cos(self.time_sec * 0.1),
                0.002 * math.sin(self.time_sec * 0.05)
            ]

            # --- Uncomment next line to inject faults ---
            # self.inject_faults()

            drop = self.nominal_altitude - self.altitude
            status = self.check_status()

            print(f"{self.time_sec}s Alt: {self.altitude:.2f}km Drop: {drop:.2f}km Status: {status}")

            self.time_sec += 1
            time.sleep(1)

if __name__ == "__main__":
    TelemetryBridge().run()
