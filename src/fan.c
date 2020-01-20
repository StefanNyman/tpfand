#include "fan.h"

#include <fcntl.h>
#include <linux/limits.h>
#include <regex.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "util.h"

char *max_temp_path = NULL;
char *input_temp_path = NULL;

void fan_cleanup() {
    if (max_temp_path != NULL) {
        free(max_temp_path);
    }
    if (input_temp_path != NULL) {
        free(input_temp_path);
    }
}

int qsrt_comparator(const void *a, const void *b) {
    if (a == NULL) {
        return 0;
    }
    if (b == NULL) {
        return -1;
    }
    return strcmp(*(char **)a, *(char **)b);
}

void find_path(char *path, regex_t rgx, char **out_path) {
    uint16_t lst_size = 10;
    char **lst = (char **)calloc(lst_size, sizeof(char *));
    if (lst == NULL) {
        fprintf(stderr, "Could not alloc list of size %d\n", lst_size);
        return;
    }
    traverse_dir(path, rgx, lst, &lst_size);
    for (uint16_t i = 0; i < lst_size; i++) {
        if (lst[i] == NULL) {
            lst_size = i;
            break;
        }
    }
    qsort(lst, lst_size, sizeof(char *), qsrt_comparator);
    char *target = lst[0];
    if (target != NULL) {
        *out_path = target;
    }
    for (uint16_t i = 1; i < lst_size; i++) {
        if (lst[i] == NULL) {
            break;
        }
        free(lst[i]);
    }
    free(lst);
}

bool find_max_temp_path() {
    if (max_temp_path != NULL) {
        return true;
    }
    regex_t rgx_max_temp;
    if (regcomp(&rgx_max_temp, "^temp[[:digit:]+]_max$", 0)) {
        DBG("could not compile regex");
        return false;
    }
    find_path(CORETEMP_PATH, rgx_max_temp, &max_temp_path);
    regfree(&rgx_max_temp);
    return max_temp_path != NULL;
}

bool find_input_temp_path() {
    if (input_temp_path != NULL) {
        return true;
    }
    regex_t rgx_temp_input;
    if (regcomp(&rgx_temp_input, "^temp[[:digit:]+]_input$", 0)) {
        DBG("could not compile regex");
        return false;
    }
    find_path(CORETEMP_PATH, rgx_temp_input, &input_temp_path);
    regfree(&rgx_temp_input);
    return input_temp_path != NULL;
}

bool fan_control_enabled() {
    FILE *fp = fopen(FAN_PATH, "r");
    if (fp != NULL) {
        fclose(fp);
        return true;
    }
    fprintf(
        stderr,
        "error: thinkpad_acpi fan_control option disabled. Cannot continue!\n");
    return false;
}

uint16_t get_temp(char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        char buf[PATH_MAX];
        sprintf(buf, "error: could not open \"%s\"", path);
        die(buf, EXIT_FAILURE);
    }
    unsigned int val;
    bool error = fscanf(fp, "%u", &val) != 1;
    fclose(fp);
    if (error) {
        char buf[PATH_MAX];
        sprintf(buf, "error: failed to parse temperature at path: \"%s\"",
                path);
        die(buf, EXIT_FAILURE);
    }
    return (uint16_t)(val / 1000.0);
}

uint16_t get_max_temp() {
    if (max_temp_path == NULL) {
        die("error: max temp path not specified.", EXIT_FAILURE);
    }
    return get_temp(max_temp_path);
}

uint16_t get_curr_temp() {
    if (input_temp_path == NULL) {
        die("error: temp input path not specified.", EXIT_FAILURE);
    }
    return get_temp(input_temp_path);
}

uint8_t compute_level(uint16_t curr_temp, tmp_direction_t dir, config_t *cfg) {
    uint8_t level = INVALID_LEVEL;
    if (dir == INC) {
        if (curr_temp < cfg->inc_levels[0].tmp) {
            level = cfg->base_level;
            goto endcl;
        }
        for (int i = N_FAN_LVLS - 1; i >= 0; i--) {
            if (curr_temp >= cfg->inc_levels[i].tmp) {
                level = cfg->inc_levels[i].lvl;
                break;
            }
        }
    }
    if (dir == DEC) {
        if (curr_temp < cfg->dec_levels[0].tmp) {
            level = cfg->base_level;
            goto endcl;
        }
        for (int i = 0; i < N_FAN_LVLS; i++) {
            if (curr_temp <= cfg->dec_levels[i].tmp) {
                level = cfg->dec_levels[i].lvl;
                break;
            }
        }
        if (level == INVALID_LEVEL) {
            level = cfg->dec_levels[N_FAN_LVLS - 1].lvl;
        }
    }
endcl:
    return level;
}

bool level_changed(uint16_t curr_temp, uint8_t prev_level, tmp_direction_t dir,
                   config_t *cfg) {
    uint8_t new_level = compute_level(curr_temp, dir, cfg);
    if (new_level == INVALID_LEVEL) {
        die("error: compute_level error broken", EXIT_FAILURE);
    }
    return new_level != prev_level;
}

void set_fan_level(uint16_t level) {
    char buf[20];
    memset(buf, 0, sizeof(buf));
    if (!snprintf(buf, 20, "level %u", level)) {
        die("error: failed to format level string", EXIT_FAILURE);
    }
    int file = open(FAN_PATH, O_WRONLY);
    if (!file) {
        die("error: could not open fan file", EXIT_FAILURE);
    }
    if (write(file, buf, strlen(buf)) == -1) {
        die("error: coult not write to fan file", EXIT_FAILURE);
    }
    close(file);
}

