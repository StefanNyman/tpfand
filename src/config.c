#include "config.h"
#include "util.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

size_t strtrim(char *str) {
    if (str == NULL || *str == '\0') {
        return 0;
    }
    char *left = str;
    char *right;
    while(isspace((unsigned char)*left)) {
        left++;
    }
    if (left != str) {
        memmove(str, left, (strlen(left) + 1));
    }
    if (*str == '\0') {
        return 0;
    }
    right = (char*)rawmemchr(str, '\0') -1;
    while (isspace((unsigned char)*right)) {
        right--;
    }
    *++right = '\0';
    return right - left;
}

void parse_ul(char *val, uint8_t *out, uint8_t default_value) {
    char *ptr;
    uint8_t parsed = (uint8_t)strtoul(val, &ptr, 10);
    if (errno == ERANGE) {
        *out = default_value;
        return;
    }
    if (ptr == val) {
        *out = default_value;
        return;
    }
    *out = parsed;
}

void parse_tmp_lvls(tmp_lvl_t *lvls, char *val) {
    char *otoken;
    char *orest = val;
    int ocntr = 0;
    while ((otoken = strtok_r(orest, " ", &orest)) && ocntr < N_FAN_LVLS) {
        char *ntoken;
        char *nrest = otoken;
        for (int i = 0; i < 2; i++) {
            ntoken = strtok_r(nrest, ":", &nrest);
            if (ntoken == NULL) {
                printf("invalid format for: %s\n", otoken);
                break;
            }
            if (i == 0) {
                parse_ul(ntoken, &(lvls[ocntr].tmp), lvls[ocntr].tmp);
                continue;
            }
            if (i == 1) {
                parse_ul(ntoken, &(lvls[ocntr].lvl), lvls[ocntr].lvl);
            }
        }
    }
}

#define BUF_SIZE 512

void read_config(config_t* cfg) { 
    _assert(cfg != NULL, "got null config"); 
    char line[BUF_SIZE];
    FILE *fp = fopen(CONFIG_PATH, "r");
    if (fp == NULL) {
        fprintf(stderr, "could not read config file at: %s\nusing defaults...\n", CONFIG_PATH);
        return;
    }
    while (fgets(line, BUF_SIZE, fp)) {
        if (line[0] == '#') {
            continue;
        }
        char *key, *val = NULL;
        size_t linelen;
        linelen = strtrim(line);
        if (linelen == 0) {
            continue;
        }
        if ((val = strchr(line, '#'))) {
            *val = '\0';
        }

        key = val = line;
        strsep(&val, "=");
        strtrim(key);

        if (val && !*val) {
            val = NULL;
        }

        if (strcmp(key, "POLLING") == 0) {
            parse_ul(val, &(cfg->poll_inter), cfg->poll_inter);
        } else if (strcmp(key, "BASE_LVL") == 0) {
            parse_ul(val, &(cfg->base_level), cfg->base_level);
        } else if (strcmp(key, "INC_TMP_LVLS") == 0) {
            parse_tmp_lvls(cfg->inc_levels, val);
        } else if (strcmp(key, "DEC_TMP_LVLS") == 0) {
            parse_tmp_lvls(cfg->dec_levels, val);
        }
    }
    fclose(fp);
}

void default_config(config_t* cfg) {
    _assert(cfg != NULL, "got null config");
    cfg->base_level = BASE_LVL;
    cfg->full_speed_temp = FULL_SPEED_TEMP;
    cfg->poll_inter = POLL_INTERVAL;
    cfg->max_temp = 80;
    tmp_lvl_t incs[7] = {{40, 0}, {45, 1}, {50, 2}, {55, 3},
                         {60, 4}, {65, 6}, {70, 7}};
    tmp_lvl_t decs[7] = {{40, 0}, {45, 2}, {50, 4}, {55, 4},
                         {60, 7}, {65, 7}, {70, 7}};
    for (int i = 0; i < N_FAN_LVLS; i++) {
        cfg->inc_levels[i] = incs[i];
        cfg->dec_levels[i] = decs[i];
    }
}
