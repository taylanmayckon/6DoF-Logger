#include "mpu6050.h"
#include <stdbool.h>
#include <stdint.h>
#include "hardware/i2c.h"

// Função para resetar e inicializar o MPU6050
void mpu6050_reset(){
    // Dois bytes para reset: primeiro o registrador, segundo o dado
    uint8_t buf[] = {0x6B, 0x80};
    i2c_write_blocking(I2C_PORT, MPU6050_I2C_ADDRESS, buf, 2, false);
    sleep_ms(100); // Aguarda reset e estabilização

    // Sai do modo sleep (registrador 0x6B, valor 0x00)
    buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, MPU6050_I2C_ADDRESS, buf, 2, false);
    sleep_ms(10); // Aguarda estabilização após acordar
}

// Função para ler os dados brutos do MPU6050
void mpu6050_read_raw(mpu6050_data_t *data){
    // Arrays de leitura
    int16_t accel[3], gyro[3];

    uint8_t buffer[6];

    // Lê aceleração a partir do registrador 0x3B (6 bytes)
    uint8_t val = 0x3B;
    i2c_write_blocking(I2C_PORT, MPU6050_I2C_ADDRESS, &val, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_I2C_ADDRESS, buffer, 6, false);

    for (int i = 0; i < 3; i++){
        accel[i] = (buffer[i * 2] << 8) | buffer[(i * 2) + 1];
    }

    // Armazena os dados de aceleração
    data->accel_x = accel[0];       
    data->accel_y = accel[1];
    data->accel_z = accel[2];

    // Lê giroscópio a partir do registrador 0x43 (6 bytes)
    val = 0x43;
    i2c_write_blocking(I2C_PORT, MPU6050_I2C_ADDRESS, &val, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_I2C_ADDRESS, buffer, 6, false);

    for (int i = 0; i < 3; i++){
        gyro[i] = (buffer[i * 2] << 8) | buffer[(i * 2) + 1];
    }

    // Armazena os dados de giroscópio
    data->gyro_x = gyro[0]; 
    data->gyro_y = gyro[1];
    data->gyro_z = gyro[2];

    // Lê temperatura a partir do registrador 0x41 (2 bytes)
    val = 0x41;
    i2c_write_blocking(I2C_PORT, MPU6050_I2C_ADDRESS, &val, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_I2C_ADDRESS, buffer, 2, false);

    data->temp = (buffer[0] << 8) | buffer[1];
}