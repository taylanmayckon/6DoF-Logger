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
} mpu6050_raw_data_t;

typedef struct{
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float pitch;
    float roll;
    float temp;
} mpu6050_data_t;

typedef struct{
    float pitch_output[2];
    float roll_output[2];
} mpu6050_filtered_t;

void mpu6050_reset();
void mpu6050_read_raw(mpu6050_raw_data_t *data);
void mpu6050_proccess_data(mpu6050_raw_data_t raw_data, mpu6050_data_t *final_data);
void mpu6050_debug_data(mpu6050_data_t data, mpu6050_filtered_t data_kf);
void mpu6050_kalmann_filter(mpu6050_data_t data, mpu6050_filtered_t *mpu6050_filtered);

#endif