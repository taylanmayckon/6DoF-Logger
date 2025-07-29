#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>
#include "mpu6050.h"

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

typedef struct{
    char const *const command;
    p_fn_t const function;
    char const *const help;
} cmd_def_t;

void run_mount();
void run_unmount();
void read_file(const char *filename);
void handle_filename(logger_file_t *logger_file);
void save_imu_data(logger_file_t *logger_file, mpu6050_data_t mpu_data);

#endif