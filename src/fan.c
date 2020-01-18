#include "fan.h"

#include <regex.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

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
