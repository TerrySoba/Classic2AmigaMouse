#ifndef AMIGA_MOUSE_H_
#define AMIGA_MOUSE_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    AXIS_VERTICAL,
    AXIS_HORIZONTAL,
} MouseAxis;

// This enum defines what the GPIO pins are used for on the Amiga side
typedef enum
{
    AMIGA_H_PULSE = 6,
    AMIGA_V_PULSE = 7,
    AMIGA_HQ_PULSE = 8,
    AMIGA_VQ_PULSE = 9,
    AMIGA_LEFT_BUTTON = 10,
    AMIGA_RIGHT_BUTTON = 11,
    AMIGA_MIDDLE_BUTTON = 12,
} AmigaOutputGPIO;

void initializeAmigaOutputGPIO();
void setAmigaOutput(AmigaOutputGPIO output, bool value);
void advanceAxisState(MouseAxis axis, uint32_t* state, int32_t direction);


#endif