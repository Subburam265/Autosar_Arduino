#ifndef OS_H
#define OS_H

#include "Std_Types.h"

// Initialization
void Os_Init() {
    Serial.begin(9600);
    Serial.println("AUTOSAR OS: Started");
}

// Universal Timer Check
// User passes the 'LastRunTime' variable, function returns true if time elapsed
boolean Os_CheckAlarm(uint32* staticTimer, uint32 intervalMs) {
    if (millis() - *staticTimer >= intervalMs) {
        *staticTimer = millis();
        return true;
    }
    return false;
}

// Macros for defining Tasks cleanly
#define TASK(TaskName) void TaskName(void)
#define TerminateTask() return

#endif