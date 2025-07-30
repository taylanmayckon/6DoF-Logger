#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>
#include "mpu6050.h"

// GPIO utilizada
#define LED_RED 13
#define LED_GREEN 11
#define LED_BLUE 12 
#define BUZZER_A 21
#define BUZZER_B 10

typedef void (*p_fn_t)();

typedef struct{
    bool run_mount;
    bool run_unmount;
    bool is_mounted;
    bool save_data;
    bool stop_saving;
    bool handle_filename;
} sdcard_cmds_t;

typedef struct{
    char filename[40];
    int index;
} logger_file_t;

enum led_states_t{
    INIT_MOUNT_SD,
    READY_FOR_SAVE,
    SAVE_READ_SD,
    UNMOUNT,
    ERROR
};

enum buzzer_states_t{
    INIT_SAVES,
    STOP_SAVES,
    MOUNT,
    B_UNMOUNT,
    IDLE
};

bool run_mount();
void run_unmount();
void read_file(const char *filename);
void handle_filename(logger_file_t *logger_file);
void save_imu_data(logger_file_t *logger_file, mpu6050_data_t mpu_data, mpu6050_filtered_t mpu_filtered_data);
void handle_rgb_led(enum led_states_t led_state, int wrap);
void handle_buzzer(enum buzzer_states_t *buzzer_state, int wrap);

#endif