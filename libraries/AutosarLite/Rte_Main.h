#ifndef RTE_MAIN_H
#define RTE_MAIN_H

#include "Std_Types.h"
#include "Mcal.h"

// --- RTE Standard Return Codes ---
#define RTE_E_OK      0x00u
#define RTE_E_LIMIT   0x05u
#define RTE_E_NO_DATA 0x06u

// --- RTE Lifecycle API ---
void Rte_Start() {
    // In a real ECU, this initializes the VFB (Virtual Function Bus)
    // On Arduino, we can use it to ensure Drivers are ready.
}

void Rte_Stop() {
    // Shutdown procedures
}

// --- GENERIC RTE MAPPING MACROS ---
// These are the "Tools" you use in your Sketch to build specific interfaces.
// They bridge the gap between "Application" and "MCAL".

// 1. Analog Inputs (Sensors)
// Usage: Rte_Call_ReadAdc(PIN_NUMBER, &variable);
#define Rte_Call_ReadAdc(Pin, DataPtr)   Adc_ReadGroup((Pin), (DataPtr))

// 2. Digital Inputs (Switches/Buttons)
// Usage: Rte_Call_ReadDio(PIN_NUMBER);
#define Rte_Call_ReadDio(Pin)            Dio_ReadChannel((Pin))

// 3. PWM Outputs (Actuators/Valves/Motor Speed)
// Usage: Rte_Call_WritePwm(PIN_NUMBER, DutyCycle);
#define Rte_Call_WritePwm(Pin, Duty)     Pwm_SetDutyCycle((Pin), (Duty))

// 4. Digital Outputs (Relays/LEDs)
// Usage: Rte_Call_WriteDio(PIN_NUMBER, State);
#define Rte_Call_WriteDio(Pin, State)    Dio_WriteChannel((Pin), (State))

#endif