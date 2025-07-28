#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

#define MPU6050_I2C_ADDRESS 0x68
#define I2C_PORT i2c0

typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    int16_t temp;
} mpu6050_data_t;

void mpu6050_reset();
void mpu6050_read_raw(mpu6050_data_t *data);

#endif