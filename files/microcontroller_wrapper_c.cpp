#include "microcontroller.h"
#include <cstring>
#include <iostream>
#include <cmath>

static Microcontroller* g_microcontroller = nullptr;

extern "C" {

// Initialize the microcontroller instance
void Microcontroller_Init() {
    if (!g_microcontroller) {
        g_microcontroller = new Microcontroller();
    }
}

// Sensor data struct matching cFS telemetry payload
typedef struct {
    float gyro[3];
    float magnetometer[3];
    float sun_angle;
    unsigned int timestamp;
    unsigned char valid;
} SensorData_t;

// Actuator command struct for output
typedef struct {
    float wheel_torques[3];
    float magnetorquer[3];
    unsigned int timestamp;
} ActuatorCommands_t;

// Process incoming sensor data (called from cFS app)
void Microcontroller_ProcessSensor(const SensorData_t* sensor) {
    if (g_microcontroller && sensor) {
        struct SensorData {
            float gyro[3];
            float magnetometer[3];
            float sun_angle;
            unsigned int timestamp;
            bool valid;
        } sensorData = {
            {sensor->gyro[0], sensor->gyro[1], sensor->gyro[2]},
            {sensor->magnetometer[0], sensor->magnetometer[1], sensor->magnetometer[2]},
            sensor->sun_angle,
            sensor->timestamp,
            sensor->valid != 0
        };

        //  Minor fix: call processSensorData BEFORE fetching actuators
        g_microcontroller->processSensorData(sensorData);

        
    }
}

// Get actuator commands (called from cFS app)
void Microcontroller_GetActuatorCommands(ActuatorCommands_t* commands) {
    if (g_microcontroller && commands) {
        ActuatorCommands actuators_cpp;
        g_microcontroller->getActuatorCommands(actuators_cpp);

        std::memcpy(commands->wheel_torques, actuators_cpp.wheel_torques, sizeof(commands->wheel_torques));
        std::memcpy(commands->magnetorquer, actuators_cpp.magnetorquer, sizeof(commands->magnetorquer));
        commands->timestamp = actuators_cpp.timestamp;
    }
}

// Cleanup microcontroller instance
void Microcontroller_Cleanup() {
    delete g_microcontroller;
    g_microcontroller = nullptr;
}

} // extern "C"
