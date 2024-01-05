#include "amiga_mouse.h"

#include "hardware/i2c.h"
#include "pico/stdlib.h"

void initializeAmigaOutputGPIO() {
    gpio_init(AMIGA_H_PULSE);
    gpio_init(AMIGA_V_PULSE);
    gpio_init(AMIGA_HQ_PULSE);
    gpio_init(AMIGA_VQ_PULSE);
    gpio_init(AMIGA_LEFT_BUTTON);
    gpio_init(AMIGA_RIGHT_BUTTON);
    gpio_init(AMIGA_MIDDLE_BUTTON);

    gpio_set_dir(AMIGA_H_PULSE, GPIO_OUT);
    gpio_set_dir(AMIGA_V_PULSE, GPIO_OUT);
    gpio_set_dir(AMIGA_HQ_PULSE, GPIO_OUT);
    gpio_set_dir(AMIGA_VQ_PULSE, GPIO_OUT);
    gpio_set_dir(AMIGA_LEFT_BUTTON, GPIO_OUT);
    gpio_set_dir(AMIGA_RIGHT_BUTTON, GPIO_OUT);
    gpio_set_dir(AMIGA_MIDDLE_BUTTON, GPIO_OUT);
}


void setAmigaOutput(AmigaOutputGPIO output, bool value) {
    gpio_put(output, value);
}


void advanceAxisState(MouseAxis axis, uint32_t* state, int32_t direction)
{
    const int32_t STATE_COUNT = 4;
    *state += direction;
    *state %= STATE_COUNT;
    switch (axis) {
        case AXIS_VERTICAL:
            switch(*state) {
            case 0:
                setAmigaOutput(AMIGA_V_PULSE, false);
                setAmigaOutput(AMIGA_VQ_PULSE, false);
                break;
            case 1:
                setAmigaOutput(AMIGA_V_PULSE, true);
                setAmigaOutput(AMIGA_VQ_PULSE, false);
                break;
            case 2:
                setAmigaOutput(AMIGA_V_PULSE, true);
                setAmigaOutput(AMIGA_VQ_PULSE, true);
                break;
            case 3:
                setAmigaOutput(AMIGA_V_PULSE, false);
                setAmigaOutput(AMIGA_VQ_PULSE, true);
                break;
            }
            break;
        case AXIS_HORIZONTAL:
            switch(*state) {
            case 0:
                setAmigaOutput(AMIGA_H_PULSE, false);
                setAmigaOutput(AMIGA_HQ_PULSE, false);
                break;
            case 1:
                setAmigaOutput(AMIGA_H_PULSE, true);
                setAmigaOutput(AMIGA_HQ_PULSE, false);
                break;
            case 2:
                setAmigaOutput(AMIGA_H_PULSE, true);
                setAmigaOutput(AMIGA_HQ_PULSE, true);
                break;
            case 3:
                setAmigaOutput(AMIGA_H_PULSE, false);
                setAmigaOutput(AMIGA_HQ_PULSE, true);
                break;
            }
            break;
    }
}
