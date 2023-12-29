#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <stdint.h>

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

bool readClassicControllerData(uint16_t* buttonData) {
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

    *buttonData = buf[4] | (buf[5] << 8);

    return true;
}


int main() {
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

    while (true) {
        uint16_t buttonData;
        ret = readClassicControllerData(&buttonData);
        if (!ret) {
            goto init;
        }
        gpio_put(LED_PIN, buttonData & 0x02);
        sleep_ms(50);
        gpio_put(LED_PIN, 0);
        sleep_ms(50);
    }

}
