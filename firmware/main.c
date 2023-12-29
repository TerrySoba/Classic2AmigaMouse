#include "pico/stdlib.h"
#include "hardware/i2c.h"



#define CLASSIC_CONTROLLER_ADDR 0x52 /* address of classic controller and nunchuck */

bool initializeClassicController() {
    unsigned char initData[2] = { 0x40, 0x00 };
    // HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(i2c, CLASSIC_CONTROLLER_ADDR << 1,
    //         initData, 2, HAL_MAX_DELAY);
    int ret = i2c_write_blocking(i2c_default, CLASSIC_CONTROLLER_ADDR, initData, 2, false);
    return ret == 0;
}



bool readClassicControllerData(uint16_t* buttonData) {
    unsigned char zeroData[1] = { 0x00 };
    // HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(i2c, CLASSIC_CONTROLLER_ADDR << 1, zeroData, 1,
    //         HAL_MAX_DELAY);
    int ret = i2c_write_blocking(i2c_default, CLASSIC_CONTROLLER_ADDR, zeroData, 1, false);
    // if (ret != HAL_OK) {
    //     debugPrintf("ret2==%d", ret);
    //     return false;
    // }

    // HAL_Delay(2);
    sleep_ms(1);

    // now receive controller data
    uint8_t buf[6];
    // ret = HAL_I2C_Master_Receive(i2c, CLASSIC_CONTROLLER_ADDR << 1, buf, 6,
    //         HAL_MAX_DELAY);
    // if (ret != HAL_OK) {
    //     debugPrintf("ret3==%d", ret);
    //     return false;
    // }
    ret = i2c_read_blocking(i2c_default, CLASSIC_CONTROLLER_ADDR, buf, 6, false);


    // now decrypt data
    for (uint8_t i = 0; i < 6; ++i) {
        buf[i] = (buf[i] ^ 0x17) + 0x17; // decrypt data
    }


    *buttonData = buf[4] | (buf[5] << 8);

    return true;
}


void blinkFast(uint pin)
{
    while (true) {
        gpio_put(pin, 1);
        sleep_ms(100);
        gpio_put(pin, 0);
        sleep_ms(100);
    }
}




int main() {
    // init LED pin
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // init I2C
    const uint I2C_PORT = PICO_DEFAULT_I2C;
    const uint I2C_SDA_PIN = PICO_DEFAULT_I2C_SDA_PIN;
    const uint I2C_SCL_PIN = PICO_DEFAULT_I2C_SCL_PIN;
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);


    bool ret = initializeClassicController();
    // if (!ret) {
    //     blinkFast(LED_PIN);
    // }

    // blinkFast(LED_PIN);

    while (true) {
        uint16_t buttonData;
        readClassicControllerData(&buttonData);
        gpio_put(LED_PIN, buttonData & 0x02);
        sleep_ms(50);
        gpio_put(LED_PIN, 0);
        sleep_ms(50);
    }

}
