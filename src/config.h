#ifndef _CONFIG_H
#define _CONFIG_H 1

#include <stdint.h>

#define N_FAN_LVLS 7
#define BASE_LVL 1
#define POLL_INTERVAL 3
#define FULL_SPEED_TEMP 70

#define PROC_PATH "/proc/acpi/ibm"
#define FAN_PATH PROC_PATH "/fan"
#define CORETEMP_PATH "/sys/devices/platform/coretemp.0"
#define HWMON_PATH CORETEMP_PATH "/hwmon"
#define CONFIG_PATH "/etc/tpfand.config"

typedef struct tmp_lvl {
    uint8_t tmp;
    uint8_t lvl;
} tmp_lvl_t;

typedef struct config {
    uint8_t base_level;
    uint8_t full_speed_temp;
    uint8_t poll_inter;
    tmp_lvl_t inc_levels[N_FAN_LVLS];
    tmp_lvl_t dec_levels[N_FAN_LVLS];
} config_t;

void read_config(config_t* cfg);
void default_config(config_t* cfg);

#endif
