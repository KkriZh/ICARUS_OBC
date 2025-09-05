#include "microcontroller.h"
#include "adcs_controller.h"
#include <iostream>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <bitset> // For fault flag visualization

Microcontroller::Microcontroller()
    : control_mode_(1), // Start in NORMAL mode
      fault_flags_(0), control_cycles_(0), uptime_seconds_(0),
      fault_threshold_(0.5f), max_torque_(0.1f),
      fault_count_(0), consecutive_faults_(0) {

    adcs_controller_ = new ADCSController(0.1f, 0.05f, 0.01f); // PID gains

    std::memset(gyro_rates_, 0, sizeof(gyro_rates_));
    std::memset(magnetometer_, 0, sizeof(magnetometer_));
    std::memset(wheel_torques_, 0, sizeof(wheel_torques_));
    std::memset(magnetorquer_, 0, sizeof(magnetorquer_));

    sun_angle_ = 0.0f;

    std::cout << "Microcontroller initialized - Normal Mode" << std::endl;
}

Microcontroller::~Microcontroller() {
    delete adcs_controller_;
}

void Microcontroller::processSensorData(const SensorData& sensor_data) {
    if (!sensor_data.valid) {
        consecutive_faults_++;
        if (consecutive_faults_ > 5) {
            control_mode_ = 0;
            std::cout << "[FAULT] Too many invalid sensor packets → SAFE MODE" << std::endl;
        }
        return;
    }

    // Normal sensor update
    for (int i = 0; i < 3; ++i) {
        gyro_rates_[i] = sensor_data.gyro[i];
        magnetometer_[i] = sensor_data.magnetometer[i];
    }
    sun_angle_ = sensor_data.sun_angle;

    // Fault Injection for demo
    
    // if (control_cycles_ == 20) {
    //     std::cout << "[FAULT INJECTION] Injecting corrupted gyro + NaN magnetometer" << std::endl;

    //     gyro_rates_[0] = 50.0f;       // Extreme value → exceeds fault_threshold_
    //     gyro_rates_[1] = NAN;         // Invalid → triggers sensor_fault
    //     gyro_rates_[2] = 20.0f;       // Also high → ensures multiple fault flags

    //     magnetometer_[2] = NAN;       // Additional fault source
    // }
    

    consecutive_faults_ = 0;

    performFaultDetection();

    adcs_controller_->computeControl(
        gyro_rates_, magnetometer_, sun_angle_,
        wheel_torques_, magnetorquer_, control_mode_
    );

    // Clamp actuator outputs
    for (int i = 0; i < 3; ++i) {
        wheel_torques_[i] = std::max(-max_torque_, std::min(max_torque_, wheel_torques_[i]));
        magnetorquer_[i] = std::max(-1.0f, std::min(1.0f, magnetorquer_[i]));
    }

    control_cycles_++;
    updateHealthMonitoring();
}

void Microcontroller::getActuatorCommands(ActuatorCommands& commands) {
    for (int i = 0; i < 3; ++i) {
        commands.wheel_torques[i] = wheel_torques_[i];
        commands.magnetorquer[i] = magnetorquer_[i];
    }
    commands.timestamp = control_cycles_;
}

void Microcontroller::setControlMode(int mode) {
    if (mode >= 0 && mode <= 3) {
        control_mode_ = mode;
        std::cout << "Control mode set to: " << mode << std::endl;
    }
}

void Microcontroller::setFaultThreshold(float threshold) {
    if (threshold > 0.0f) {
        fault_threshold_ = threshold;
    }
}

void Microcontroller::performFaultDetection() {
    fault_flags_ = 0;

    // Threshold-based fault detection
    for (int i = 0; i < 3; ++i) {
        if (std::abs(gyro_rates_[i]) > fault_threshold_) {
            fault_flags_ |= (1 << i); // Set bit 0,1,2 for X,Y,Z gyro faults
        }
    }

    // NaN/Inf detection
    bool sensor_fault = false;
    for (int i = 0; i < 3; ++i) {
        if (std::isnan(gyro_rates_[i]) || std::isinf(gyro_rates_[i]) ||
            std::isnan(magnetometer_[i]) || std::isinf(magnetometer_[i])) {
            sensor_fault = true;
            break;
        }
    }

    if (sensor_fault) {
        fault_flags_ |= 0x08; // Bit 3 → general sensor fault
    }

    // Log fault flags
    if (fault_flags_ != 0) {
        fault_count_++;
        std::cout << "[FAULT DETECTED] Flags: " << std::bitset<8>(fault_flags_) << std::endl;

        if (fault_count_ > 2 && control_mode_ != 0) {
            control_mode_ = 0;
            std::cout << "[FAULT DETECTED] Multiple sensor anomalies → SAFE MODE" << std::endl;
        }
    } else {
        fault_count_ = 0;
    }
}

void Microcontroller::updateHealthMonitoring() {
    if (control_cycles_ % 10 == 0) {
        uptime_seconds_++;
    }
}
