#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define CLASSIC_CONTROLLER_ADDR 0x52 /* address of classic controller and nunchuck */

#define DEAD_ZONE_SIZE 4
#define ANALOG_DIVIDER 4
#define CYCLES_PER_INTERVAL_DIGITAL 10



bool initializeClassicController() {
    const uint8_t initData[] = { 0x40, 0x00 };
    const size_t initDataSize = sizeof(initData) / sizeof(initData[0]);
    int ret = i2c_write_blocking(i2c_default, CLASSIC_CONTROLLER_ADDR, initData, initDataSize, false);
    if (ret != initDataSize) {
        return false;
    }
    return true;
}

typedef struct ClassicControllerData_ {
    uint16_t buttonData;
    int8_t leftStickX;
    int8_t leftStickY;
} ClassicControllerData;


int32_t applyDeadzone(int32_t value, uint32_t deadzone) {
    if (value > 0) {
        value -= deadzone;
        if (value < 0) {
            value = 0;
        }
    }
    else if (value < 0) {
        value += deadzone;
        if (value > 0) {
            value = 0;
        }
    }
    return value;
}


bool readClassicControllerData(ClassicControllerData* controllerData) {
    const uint8_t zeroData[] = { 0x00 };
    const size_t zeroDataSize = sizeof(zeroData) / sizeof(zeroData[0]);
    int ret = i2c_write_blocking(i2c_default, CLASSIC_CONTROLLER_ADDR, zeroData, zeroDataSize, false);
    if (ret != zeroDataSize) {
        return false;
    }

    sleep_ms(1);

    // now receive controller data
    uint8_t buf[6];
    ret = i2c_read_blocking(i2c_default, CLASSIC_CONTROLLER_ADDR, buf, 6, false);
    if (ret != 6) {
        return false;
    }

    // now decrypt data
    for (uint8_t i = 0; i < 6; ++i) {
        buf[i] = (buf[i] ^ 0x17) + 0x17; // decrypt data
    }

    controllerData->buttonData = buf[4] | (buf[5] << 8);


    controllerData->leftStickX = (buf[0] & 0x3F) - 32;
    controllerData->leftStickY = (buf[1] & 0x3F) - 32;
    
    return true;
}

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

typedef enum
{
    AXIS_VERTICAL,
    AXIS_HORIZONTAL,
} MouseAxis;

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


bool buttonPressed(ClassicButtons button, ClassicControllerData* controllerData) {
    return !(controllerData->buttonData & button);
}


int main() {
    stdio_init_all();
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

    init:
    sleep_ms(100);
    ret = initializeClassicController();
    if (!ret) {
        goto init;
    }


    int32_t xAxis = 0;
    int32_t yAxis = 0;
    uint32_t xAxisState = 0;
    uint32_t yAxisState = 0;

    while (true) {
        ClassicControllerData controllerData;
        ret = readClassicControllerData(&controllerData);
        if (!ret) {
            goto init;
        }
        gpio_put(LED_PIN, controllerData.buttonData & CLASSIC_BTN_a);
        

        setAmigaOutput(AMIGA_LEFT_BUTTON, buttonPressed(CLASSIC_BTN_y, &controllerData));
        setAmigaOutput(AMIGA_MIDDLE_BUTTON, buttonPressed(CLASSIC_BTN_a, &controllerData));
        setAmigaOutput(AMIGA_RIGHT_BUTTON, buttonPressed(CLASSIC_BTN_b, &controllerData));


        // handle digital joystick
        if (buttonPressed(CLASSIC_BTN_left, &controllerData)) {
            xAxis -= 1;
        }
        if (buttonPressed(CLASSIC_BTN_right, &controllerData)) {
            xAxis += 1;
        }
        if (buttonPressed(CLASSIC_BTN_up, &controllerData)) {
            yAxis -= 1;
        }
        if (buttonPressed(CLASSIC_BTN_down, &controllerData)) {
            yAxis += 1;
        }
       

        // handle analog joystick
        int32_t analogX = applyDeadzone(controllerData.leftStickX, DEAD_ZONE_SIZE) / ANALOG_DIVIDER;
        int32_t analogY = applyDeadzone(controllerData.leftStickY, DEAD_ZONE_SIZE) / ANALOG_DIVIDER;
        xAxis += analogX;
        yAxis += analogY;


        const int32_t CYCLES_PER_INTERVAL = CYCLES_PER_INTERVAL_DIGITAL;

        if (xAxis > CYCLES_PER_INTERVAL) {
            xAxis -= CYCLES_PER_INTERVAL;
            advanceAxisState(AXIS_HORIZONTAL, &xAxisState, 1);
        }
        if (yAxis > CYCLES_PER_INTERVAL) {
            yAxis -= CYCLES_PER_INTERVAL;
            advanceAxisState(AXIS_VERTICAL, &yAxisState, 1);
        }
        if (xAxis < -CYCLES_PER_INTERVAL) {
            xAxis += CYCLES_PER_INTERVAL;
            advanceAxisState(AXIS_HORIZONTAL, &xAxisState, -1);
        }
        if (yAxis < -CYCLES_PER_INTERVAL) {
            yAxis += CYCLES_PER_INTERVAL;
            advanceAxisState(AXIS_VERTICAL, &yAxisState, -1);
        }

        // printf("xAxis: %6d, yAxis: %6d\n", xAxis, yAxis);
        printf("x: %6d, y: %6d\n", analogX, analogY);
            

        sleep_ms(1);
    }

}
