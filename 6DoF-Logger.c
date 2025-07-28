#include <stdio.h>
#include "pico/stdlib.h"
#include "mpu6050.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

// Definição dos pinos I2C para o MPU6050
#define I2C_PORT i2c0                 
#define I2C_SDA 0
#define I2C_SCL 1

// Definição dos pinos I2C para o display OLED
#define I2C_PORT_DISP i2c1
#define I2C_SDA_DISP 14
#define I2C_SCL_DISP 15
#define ENDERECO_DISP 0x3C            

// Struct do MPU6050
mpu6050_data_t mpu_data;

int main(){
    stdio_init_all();
    sleep_ms(5000);

    // Inicialização da I2C do MPU6050
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Declara os pinos como I2C na Binary Info
    bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
    mpu6050_reset();

    while (true){
        // Lê os dados do MPU6050
        mpu6050_read_raw(&mpu_data);

        // Debug
        printf("[ACELERACAO (g)]\n");
        printf("X: %d | Y: %d | Z: %d\n", mpu_data.accel_x/16384, mpu_data.accel_y/16384, mpu_data.accel_z/16384);
        printf("[GIROSCOPIO (dps)]\n");
        printf("X: %d | Y: %d | Z: %d\n\n", mpu_data.gyro_x/131, mpu_data.gyro_y/131, mpu_data.gyro_z/131);
        sleep_ms(1000);
    }
}
