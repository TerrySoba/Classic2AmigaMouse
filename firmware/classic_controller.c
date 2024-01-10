#include "classic_controller.h"

#include "hardware/i2c.h"


#define CLASSIC_CONTROLLER_ADDR 0x52 /* address of classic controller and nunchuck */

bool initializeClassicController() {
    const uint8_t initData[] = { 0x40, 0x00 };
    const size_t initDataSize = sizeof(initData) / sizeof(initData[0]);
    int ret = i2c_write_blocking(i2c_default, CLASSIC_CONTROLLER_ADDR, initData, initDataSize, false);
    if (ret != initDataSize) {
        return false;
    }
    return true;
}

bool readClassicControllerData(ClassicControllerData* controllerData) {
    const uint8_t zeroData[] = { 0x00 };
    const size_t zeroDataSize = sizeof(zeroData) / sizeof(zeroData[0]);
    int ret = i2c_write_blocking(i2c_default, CLASSIC_CONTROLLER_ADDR, zeroData, zeroDataSize, false);
    if (ret != zeroDataSize) {
        return false;
    }

    sleep_us(300);

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

bool buttonPressed(ClassicButtons button, const ClassicControllerData* controllerData) {
    return !(controllerData->buttonData & button);
}
