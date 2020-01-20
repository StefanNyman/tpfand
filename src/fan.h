#ifndef _FAN_H
#define _FAN_H 1
#include <stdbool.h>
#include <stdint.h>

#include "config.h"

void fan_cleanup();
bool find_max_temp_path();
bool find_input_temp_path();
bool fan_control_enabled();

uint16_t get_max_temp();
uint16_t get_curr_temp();

uint8_t compute_level(uint16_t curr_temp, tmp_direction_t dir, config_t* cfg);
bool level_changed(uint16_t curr_temp, uint8_t prev_level, tmp_direction_t dir,
                   config_t* cfg);
void set_fan_level(uint16_t level);

#endif
