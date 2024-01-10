#include "classic_controller.h"
#include "amiga_mouse.h"
#include "button_config.h"

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define CYCLES_PER_INTERVAL_DIGITAL 4
#define I2C_POLL_DIVIDER 4

#define toSubpixel(x) ((x) << 4)
#define toPixel(x) ((x) >> 4)

static const int32_t MOUSE_SPEED_MAP[] = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 4, 4, 5, 6, 7, 8, 16, 16, 16, 16, 32, 32, 32, 32, 32, 32, 32 };
static const int32_t MOUSE_SPEED_MAP_SIZE = sizeof(MOUSE_SPEED_MAP) / sizeof(MOUSE_SPEED_MAP[0]);


int32_t integerSqrt(int32_t x) {
    int32_t y = 0;
    int32_t b = 0x8000;
    while (b > 0) {
        if (x >= y + b) {
            x -= y + b;
            y = (y >> 1) + b;
        } else {
            y >>= 1;
        }
        b >>= 2;
    }
    return y;
}


int32_t mapMouseSpeed(int32_t value) {
    int32_t sign = value < 0 ? -1 : 1;
    value = abs(value);

    int32_t out = 0;
    if (value >= MOUSE_SPEED_MAP_SIZE) {
        out = MOUSE_SPEED_MAP[MOUSE_SPEED_MAP_SIZE - 1];
    } else {
        out = MOUSE_SPEED_MAP[value];
    }
    return sign * out;
}


typedef struct AxisState_
{
    int32_t xAxis;
    int32_t yAxis;
    uint32_t xAxisState;
    uint32_t yAxisState;
} AxisState;

void handleMouseMode(const ClassicControllerData* controllerData, AxisState* axisState)
{
    setAmigaOutput(AMIGA_LEFT_BUTTON, buttonPressed(CLASSIC_CONTROLLER_LEFT_AMIGA_MOUSE_BUTTON, controllerData));
    setAmigaOutput(AMIGA_MIDDLE_BUTTON, buttonPressed(CLASSIC_CONTROLLER_MIDDLE_AMIGA_MOUSE_BUTTON, controllerData));
    setAmigaOutput(AMIGA_RIGHT_BUTTON, buttonPressed(CLASSIC_CONTROLLER_RIGHT_AMIGA_MOUSE_BUTTON, controllerData));

    int speedRightShift = 0;

    if (buttonPressed(CLASSIC_CONTROLLER_HALF_SPEED_BUTTON, controllerData))
    {
        speedRightShift = 1;
    }
    else if (buttonPressed(CLASSIC_CONTROLLER_QUATER_SPEED_BUTTON, controllerData))
    {
        speedRightShift = 2;
    }

    // handle digital joystick
    if (buttonPressed(CLASSIC_BTN_left, controllerData))
    {
        axisState->xAxis -= toSubpixel(1) >> speedRightShift;
    }
    if (buttonPressed(CLASSIC_BTN_right, controllerData))
    {
        axisState->xAxis += toSubpixel(1) >> speedRightShift;
    }
    if (buttonPressed(CLASSIC_BTN_up, controllerData))
    {
        axisState->yAxis -= toSubpixel(1) >> speedRightShift;
    }
    if (buttonPressed(CLASSIC_BTN_down, controllerData))
    {
        axisState->yAxis += toSubpixel(1) >> speedRightShift;
    }

    // handle analog joystick

    /// calculate length of speed vector
    int32_t len = integerSqrt(
        controllerData->leftStickX * controllerData->leftStickX +
        controllerData->leftStickY * controllerData->leftStickY);

    /// map length to speed
    uint32_t speed = mapMouseSpeed(len);

    /// calculate x and y components of speed vector
    uint32_t xSpeed = (uint32_t)((int64_t)speed * controllerData->leftStickX / len);
    uint32_t ySpeed = (uint32_t)((int64_t)speed * controllerData->leftStickY / len);

    axisState->xAxis += xSpeed; // mapMouseSpeed(controllerData.leftStickX);
    axisState->yAxis -= ySpeed; // mapMouseSpeed(controllerData.leftStickY);

    const int32_t CYCLES_PER_INTERVAL = toSubpixel(CYCLES_PER_INTERVAL_DIGITAL);

    if (axisState->xAxis > CYCLES_PER_INTERVAL)
    {
        axisState->xAxis -= CYCLES_PER_INTERVAL;
        advanceAxisState(AXIS_HORIZONTAL, &axisState->xAxisState, 1);
    }
    if (axisState->yAxis > CYCLES_PER_INTERVAL)
    {
        axisState->yAxis -= CYCLES_PER_INTERVAL;
        advanceAxisState(AXIS_VERTICAL, &axisState->yAxisState, 1);
    }
    if (axisState->xAxis < -CYCLES_PER_INTERVAL)
    {
        axisState->xAxis += CYCLES_PER_INTERVAL;
        advanceAxisState(AXIS_HORIZONTAL, &axisState->xAxisState, -1);
    }
    if (axisState->yAxis < -CYCLES_PER_INTERVAL)
    {
        axisState->yAxis += CYCLES_PER_INTERVAL;
        advanceAxisState(AXIS_VERTICAL, &axisState->yAxisState, -1);
    }
}

void handleJoystickMode(const ClassicControllerData* controllerData, bool jumpAndRunMode)
{
    if (jumpAndRunMode)
    {
        setAmigaOutput(AMIGA_JOYSTICK_BUTTON,
            buttonPressed(CLASSIC_CONTROLLER_JOYSTICK_ACTION_BUTTON_A, controllerData) ||
            buttonPressed(CLASSIC_CONTROLLER_JOYSTICK_ACTION_BUTTON_B, controllerData));
        setAmigaOutput(AMIGA_JOYSTICK_UP,
            buttonPressed(CLASSIC_CONTROLLER_JOYSTICK_UP, controllerData) ||
            buttonPressed(CLASSIC_CONTROLLER_JOYSTICK_JUMP_BUTTON_A, controllerData) ||
            buttonPressed(CLASSIC_CONTROLLER_JOYSTICK_JUMP_BUTTON_B, controllerData));
    }
    else
    {
        setAmigaOutput(AMIGA_JOYSTICK_BUTTON,
            buttonPressed(CLASSIC_CONTROLLER_JOYSTICK_ACTION_BUTTON_A, controllerData) ||
            buttonPressed(CLASSIC_CONTROLLER_JOYSTICK_ACTION_BUTTON_B, controllerData) ||
            buttonPressed(CLASSIC_CONTROLLER_JOYSTICK_JUMP_BUTTON_A, controllerData) ||
            buttonPressed(CLASSIC_CONTROLLER_JOYSTICK_JUMP_BUTTON_B, controllerData));
        setAmigaOutput(AMIGA_JOYSTICK_UP, buttonPressed(CLASSIC_CONTROLLER_JOYSTICK_UP, controllerData));
    }

    setAmigaOutput(AMIGA_JOYSTICK_LEFT, buttonPressed(CLASSIC_CONTROLLER_JOYSTICK_LEFT, controllerData));
    setAmigaOutput(AMIGA_JOYSTICK_RIGHT, buttonPressed(CLASSIC_CONTROLLER_JOYSTICK_RIGHT, controllerData));
    setAmigaOutput(AMIGA_JOYSTICK_DOWN, buttonPressed(CLASSIC_CONTROLLER_JOYSTICK_DOWN, controllerData));
}

typedef enum AdapterMode_
{
	MOUSE_MODE,
    JOYSTICK_MODE,
	JUMP_RUN_MODE,
} AdapterMode;

void checkModeChange(const ClassicControllerData* controllerData, AdapterMode* mode)
{
    if (buttonPressed(CLASSIC_BTN_select, controllerData) && buttonPressed(CLASSIC_BTN_up, controllerData))
    {
        *mode = JUMP_RUN_MODE;
    }
    else if (buttonPressed(CLASSIC_BTN_select, controllerData) && buttonPressed(CLASSIC_BTN_down, controllerData))
    {
        *mode = JOYSTICK_MODE;
    }
    else if (buttonPressed(CLASSIC_BTN_select, controllerData) && buttonPressed(CLASSIC_BTN_left, controllerData))
    {
        *mode = MOUSE_MODE;
    }
}

int main() {

    uint8_t counter = 0;

    // stdio_init_all();
    initializeAmigaOutputGPIO();

    // init LED pin
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // init I2C
    const uint I2C_SDA_PIN = PICO_DEFAULT_I2C_SDA_PIN;
    const uint I2C_SCL_PIN = PICO_DEFAULT_I2C_SCL_PIN;
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);


    bool ret;
    AdapterMode mode = MOUSE_MODE;

    init:
    
    sleep_ms(100);
    ret = initializeClassicController();
    if (!ret) {
        goto init;
    }


    AxisState axisState;
    axisState.xAxis = 0;
    axisState.yAxis = 0;
    axisState.xAxisState = 0;
    axisState.yAxisState = 0;

    ClassicControllerData controllerData;

    while (true) {
        if (counter % I2C_POLL_DIVIDER == 0)
        {
            ret = readClassicControllerData(&controllerData);
            if (!ret) {
                goto init;
            }
        }
        gpio_put(LED_PIN, controllerData.buttonData & CLASSIC_BTN_a);
        ++counter;

        checkModeChange(&controllerData, &mode);

        switch (mode)
        {
        case MOUSE_MODE:
            handleMouseMode(&controllerData, &axisState);
            break;

        case JOYSTICK_MODE:
            handleJoystickMode(&controllerData, false);
            break;

        case JUMP_RUN_MODE:
            handleJoystickMode(&controllerData, true);
            break;
        }

        sleep_us(100);
    }

}
