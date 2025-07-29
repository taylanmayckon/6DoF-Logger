#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>

typedef void (*p_fn_t)();

typedef struct{
    bool run_mount;
    bool run_unmount;
    bool is_mounted;
} sdcard_cmds_t;

typedef struct{
    char const *const command;
    p_fn_t const function;
    char const *const help;
} cmd_def_t;

void run_mount();
void run_unmount();
void read_file(const char *filename);

#endif