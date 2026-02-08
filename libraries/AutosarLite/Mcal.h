#ifndef MCAL_H
#define MCAL_H

#include "Std_Types.h"

// --- DIO DRIVER (Digital I/O) ---
// Universal: User passes the pin number (ChannelId)
void Dio_WriteChannel(uint8 ChannelId, uint8 Level) {
    digitalWrite(ChannelId, (Level == STD_HIGH ? HIGH : LOW));
}

uint8 Dio_ReadChannel(uint8 ChannelId) {
    return digitalRead(ChannelId);
}

void Dio_FlipChannel(uint8 ChannelId) {
    digitalWrite(ChannelId, !digitalRead(ChannelId));
}

// --- ADC DRIVER (Analog Input) ---
typedef uint16 Adc_ValueGroupType;

Std_ReturnType Adc_ReadGroup(uint8 GroupId, Adc_ValueGroupType* DataBufferPtr) {
    // GroupId maps directly to Arduino Analog Pins (A0, A1, etc.)
    *DataBufferPtr = analogRead(GroupId); 
    return E_OK;
}

// --- PWM DRIVER (Actuator Control) ---
void Pwm_SetDutyCycle(uint8 ChannelId, uint16 DutyCycle) {
    // Expects DutyCycle 0-255
    analogWrite(ChannelId, (int)DutyCycle);
}

// --- PORT DRIVER (Setup) ---
// User calls this to set pin directions
void Port_SetPinDirection(uint8 Pin, uint8 Direction) {
    pinMode(Pin, Direction);
}

#endif