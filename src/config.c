#include "config.h"

#include "util.h"

void read_config(config_t* cfg) { _assert(cfg != NULL, "got null config"); }

void default_config(config_t* cfg) {
    _assert(cfg != NULL, "got null config");
    cfg->base_level = BASE_LVL;
    cfg->full_speed_temp = FULL_SPEED_TEMP;
    cfg->poll_inter = POLL_INTERVAL;
    tmp_lvl_t incs[7] = {{40, 0}, {45, 1}, {50, 2}, {55, 3},
                         {60, 4}, {65, 6}, {70, 7}};
    tmp_lvl_t decs[7] = {{40, 0}, {45, 1}, {50, 2}, {55, 3},
                         {60, 6}, {65, 7}, {70, 7}};
    for (int i = 0; i < N_FAN_LVLS; i++) {
        cfg->inc_levels[i] = incs[i];
        cfg->dec_levels[i] = decs[i];
    }
}
