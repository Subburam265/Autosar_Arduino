#ifndef STD_TYPES_H
#define STD_TYPES_H

#include <Arduino.h>

typedef uint8_t   uint8;
typedef uint16_t  uint16;
typedef uint32_t  uint32;
typedef int8_t    sint8;
typedef int16_t   sint16;
typedef int32_t   sint32;
typedef uint8     Std_ReturnType;

#define E_OK      0x00u
#define E_NOT_OK  0x01u
#define STD_HIGH  0x01u
#define STD_LOW   0x00u

#endif