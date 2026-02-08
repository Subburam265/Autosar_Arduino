/* * PROJECT: Physics-Based ABS & Cruise Control Simulation
 * STACK: AutosarCore (Generic Library)
 * HARDWARE: Arduino Mega / Uno
 */

#include <Std_Types.h>
#include <Mcal.h>
#include <Os.h>
#include <Rte_Main.h> 

// ==========================================
// 1. HARDWARE CONFIGURATION
// ==========================================
#define PIN_BRAKE_INPUT   A0   // Potentiometer (Brake Pedal)
#define PIN_BRAKE_OUTPUT  4    // LED 1 (Brake Light / ABS Action)
#define PIN_SPEED_VISUAL  5   // LED 2 (Speedometer Visualizer)

// ==========================================
// 2. PHYSICS TUNING CONSTANTS
// ==========================================
#define MASS_INERTIA       5.0f   // Car Weight (Higher = Slower changes)
#define GAIN_P             0.5f   // Braking Strength (Proportional)
#define GAIN_D             2.0f   // Slam Sensitivity (Derivative)

#define LOG_DECAY_FACTOR   0.005f // Curve of deceleration
#define ACCEL_FACTOR       0.8f   // Engine Power (Recovery speed)

// ==========================================
// 3. RTE INTERFACE LAYER
// ==========================================
// Maps App Logic <---> Hardware Pins

Std_ReturnType Rte_Read_RpBrakePressure(uint16* value) {
    return Rte_Call_ReadAdc(PIN_BRAKE_INPUT, value);
}

Std_ReturnType Rte_Write_PpBrakeActuator(uint8 value) {
    Rte_Call_WritePwm(PIN_BRAKE_OUTPUT, value);
    return RTE_E_OK;
}

Std_ReturnType Rte_Write_PpSpeedIndicator(uint8 value) {
    Rte_Call_WritePwm(PIN_SPEED_VISUAL, value);
    return RTE_E_OK;
}

// ==========================================
// 4. APPLICATION LAYER (Physics Engine)
// ==========================================

// Global State Variables
float Swc_VehicleSpeed = 0.0f;     // Current Actual Speed
float Swc_TargetSpeed = 0.0f;      // Desired Cruise Speed (Set via Serial)
float Swc_LastBrakeInput = 0.0f;   // For derivative calc

TASK(VehicleDynamicsRunnable) {
    uint16 rawBrake;
    
    // A. READ INPUTS
    Rte_Read_RpBrakePressure(&rawBrake);
    float currentBrake = map(rawBrake, 0, 1023, 0, 100);

    // B. CALCULATE BRAKING EFFORT
    float brakeRate = currentBrake - Swc_LastBrakeInput;
    Swc_LastBrakeInput = currentBrake;
    float brakeEffort = (currentBrake * GAIN_P) + (brakeRate * GAIN_D);
    if (brakeEffort < 0) brakeEffort = 0;

    // C. PHYSICS ENGINE (Tri-Mode: Brake, Accel, Coast)
    // -----------------------------------------------------------
    
    // MODE 1: ACTIVE BRAKING (Priority 1)
    // Condition: Driver is pressing the brake pedal
    if (brakeEffort > 1.0f) {
        if (Swc_VehicleSpeed > 0) {
            float logDrop = (brakeEffort / MASS_INERTIA) * (Swc_VehicleSpeed * LOG_DECAY_FACTOR);
            float totalDrop = logDrop + (brakeEffort * 0.02f); 
            Swc_VehicleSpeed -= totalDrop;
        }
        // If we brake hard, we might drop below target. 
        // Mode 2 will pick it back up once we release the brake.
    }
    
    // MODE 2: RE-ACCELERATION (Cruise Recovery)
    // Condition: Brake Released AND Speed is TOO LOW
    else if (Swc_VehicleSpeed < Swc_TargetSpeed) {
        float speedDiff = Swc_TargetSpeed - Swc_VehicleSpeed;
        float acceleration = (ACCEL_FACTOR / MASS_INERTIA) + (speedDiff * 0.01f);
        Swc_VehicleSpeed += acceleration;
        
        // Clamp to target so we don't overshoot
        if (Swc_VehicleSpeed > Swc_TargetSpeed) Swc_VehicleSpeed = Swc_TargetSpeed;
    }
    
    // MODE 3: COASTING / ENGINE BRAKING (The Fix)
    // Condition: Brake Released AND Speed is TOO HIGH
    // (e.g., You changed Serial Target from 100 -> 50)
    else if (Swc_VehicleSpeed > Swc_TargetSpeed) {
        // Simulate Air Drag / Engine Braking
        // This is a gentle slowdown, much softer than active braking.
        float dragForce = 1.2f + (Swc_VehicleSpeed * 0.001f); 
        
        Swc_VehicleSpeed -= dragForce;

        // Clamp to target so we don't undershoot
        if (Swc_VehicleSpeed < Swc_TargetSpeed) Swc_VehicleSpeed = Swc_TargetSpeed;
    }
    // -----------------------------------------------------------

    // Clamp Speed Limits (0-255)
    if (Swc_VehicleSpeed < 0) Swc_VehicleSpeed = 0;
    if (Swc_VehicleSpeed > 255) Swc_VehicleSpeed = 255;

    // D. WRITE OUTPUTS
    Rte_Write_PpSpeedIndicator((uint8)Swc_VehicleSpeed);

    float ledBrightness = brakeEffort * 2.55; 
    if (ledBrightness > 255) ledBrightness = 255;
    Rte_Write_PpBrakeActuator((uint8)ledBrightness);
    
    TerminateTask();
}

// ==========================================
// 5. BSW / OS (Com Stack & Scheduler)
// ==========================================
uint32 task_timer = 0;

void CheckSerialCom() {
    // "Com Stack": Reads Serial to set TARGET Speed
    if (Serial.available() > 0) {
        int newTarget = Serial.parseInt();
        
        // Only update if valid number sent
        if (newTarget > 0) {
            Swc_TargetSpeed = (float)newTarget;
            
            // Optional: Jumpstart engine if stalled at 0
            if (Swc_VehicleSpeed < 1.0f) Swc_VehicleSpeed = 1.0f; 
            
            Serial.print("COM: Cruise Target Set to ");
            Serial.println(Swc_TargetSpeed);
            
            // Flush buffer
            while(Serial.available()) Serial.read();
        }
    }
}

void setup() {
    Os_Init();
    
    // Hardware Init
    Port_SetPinDirection(PIN_BRAKE_OUTPUT, OUTPUT);
    Port_SetPinDirection(PIN_SPEED_VISUAL, OUTPUT);
    Port_SetPinDirection(PIN_BRAKE_INPUT, INPUT);
    
    Serial.println("--- ABS & CRUISE SIMULATION STARTED ---");
    Serial.println("1. Enter Target Speed (e.g. 255) in Serial Monitor.");
    Serial.println("2. Use Potentiometer to Brake.");
}

void loop() {
    // 1. Run Communication Stack
    CheckSerialCom();

    // 2. Run Application Task (Every 50ms)
    // 50ms gives the "Mass" inertia calculation time to feel realistic
    if (Os_CheckAlarm(&task_timer, 50)) {
        VehicleDynamicsRunnable();
    }
}