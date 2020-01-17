#ifndef _FAN_H
#define _FAN_H 1
#include <stdbool.h>

void fan_cleanup();
bool find_max_temp_path();
bool find_input_temp_path();
bool fan_control_enabled();

#endif
