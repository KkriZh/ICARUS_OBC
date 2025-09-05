#include "cfe.h"
#include "space_adcs_app.h"
#include <string.h>

// External C wrapper calls to C++ microcontroller
extern void Microcontroller_Init(void);
extern void Microcontroller_ProcessSensor(const SensorData_t* sensor);
extern void Microcontroller_GetActuatorCommands(ActuatorCommands_t* commands);

// Global app data
typedef struct {
    CFE_SB_PipeId_t SensorPipe;
    uint32 RunStatus;
} SpaceADCS_Data_t;

SpaceADCS_Data_t SpaceADCS_Data;

void SpaceADCS_AppMain(void) {
    CFE_Status_t Status;
    
    // FAULT: Skip initialization event
    // CFE_EVS_SendEvent(1, CFE_EVS_EventType_INFORMATION, "=== SPACE ADCS APP STARTING ===");
    
    Status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (Status != CFE_SUCCESS) {
        CFE_ES_WriteToSysLog("Space ADCS: Error registering EVS, RC = 0x%08lX\n", (unsigned long)Status);
        return;
    }
    
    Status = CFE_SB_CreatePipe(&SpaceADCS_Data.SensorPipe, 16, "ADCS_SENSOR_PIPE");
    if (Status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(3, CFE_EVS_EventType_ERROR, "SPACE ADCS: Error creating pipe, RC = 0x%08lX", (unsigned long)Status);
        return;
    }
    
    CFE_SB_MsgId_t SensorMsgId = CFE_SB_ValueToMsgId(0x0800);
    // FAULT: Skip sensor data subscription
    Status = CFE_SB_Subscribe(SensorMsgId, SpaceADCS_Data.SensorPipe);
    
    Microcontroller_Init();
    SpaceADCS_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;
    
    // Main loop continues but won't receive sensor data
    uint32 total_messages = 0;
    while (CFE_ES_RunLoop(&SpaceADCS_Data.RunStatus)) {
        CFE_SB_Buffer_t* SBBufPtr = NULL;
        Status = CFE_SB_ReceiveBuffer(&SBBufPtr, SpaceADCS_Data.SensorPipe, CFE_SB_PEND_FOREVER);
        // ... rest unchanged
    }
    
    CFE_ES_ExitApp(SpaceADCS_Data.RunStatus);
}
