#ifndef CLASSIC_CONTROLLER_H_
#define CLASSIC_CONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    CLASSIC_BTN_rT     = 1 << 1,
    CLASSIC_BTN_start  = 1 << 2,
    CLASSIC_BTN_home   = 1 << 3,
    CLASSIC_BTN_select = 1 << 4,
    CLASSIC_BTN_lT     = 1 << 5,
    CLASSIC_BTN_down   = 1 << 6,
    CLASSIC_BTN_right  = 1 << 7,
    CLASSIC_BTN_up     = 1 << 8,
    CLASSIC_BTN_left   = 1 << 9,
    CLASSIC_BTN_rZ     = 1 << 10,
    CLASSIC_BTN_x      = 1 << 11,
    CLASSIC_BTN_a      = 1 << 12,
    CLASSIC_BTN_y      = 1 << 13,
    CLASSIC_BTN_b      = 1 << 14,
    CLASSIC_BTN_lZ     = 1 << 15,
} ClassicButtons;


typedef struct ClassicControllerData_ {
    uint16_t buttonData;
    int8_t leftStickX;
    int8_t leftStickY;
} ClassicControllerData;


bool initializeClassicController();
bool readClassicControllerData(ClassicControllerData* controllerData);
bool buttonPressed(ClassicButtons button, ClassicControllerData* controllerData);

#endif
