#include "fan.h"

#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "util.h"

char* max_temp_path = NULL;
char* input_temp_path = NULL;

void fan_cleanup() {
    if (max_temp_path != NULL) {
        free(max_temp_path);
    }
    if (input_temp_path != NULL) {
        free(input_temp_path);
    }
}

char* join_path(char* path, char* append) {
    if (path == NULL || append == NULL) {
        DBG("pointers invalid");
        return NULL;
    }
    int psize = strlen(path);
    int asize = strlen(append);
    int size = psize + asize + 2;
    char* buf = calloc(1, size + sizeof(char));
    if (buf == NULL) {
        fprintf(stderr, "Could not allocate buffer of size: %d\n", size);
        exit(1);
    }
    memcpy(buf, path, strlen(path));
    memcpy(buf + psize, "/", 1);
    memcpy(buf + psize + 1, append, asize);
    return buf;
}

char* traverse_dir(char* path, regex_t patt) {
    char* target = NULL;
    struct dirent* de;
    DIR* dr = opendir(path);
    if (dr == NULL) {
        DBGF("failed to open path: %s\n", path);
        return false;
    }
    while ((de = readdir(dr)) != NULL) {
        printf("%s\n", de->d_name);
        switch (de->d_type) {
            case DT_DIR: {
                if (strncmp(de->d_name, ".", 1) == 0 ||
                    strncmp(de->d_name, "..", 2) == 0) {
                    continue;
                }
                char* npath = join_path(path, de->d_name);
                target = traverse_dir(npath, patt);
                free(npath);
                if (target != NULL) {
                    goto cleanup;
                }
                break;
            }

            case DT_REG: {
                int reti = regexec(&patt, de->d_name, 0, NULL, 0);
                if (!reti) {
                    printf("%s matches\n", de->d_name);
                    target = join_path(path, de->d_name);
                    goto cleanup;
                } else {
                    char msgbuf[100];
                    regerror(reti, &patt, msgbuf, sizeof(msgbuf));
                    printf("regex match failed: %s\n", msgbuf);
                }
                break;
            }

            default:
                continue;
        }
    }
cleanup:
    closedir(dr);
    return target;
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
    char* target = traverse_dir(CORETEMP_PATH, rgx_max_temp);
    regfree(&rgx_max_temp);
    if (target != NULL) {
        max_temp_path = target;
        return true;
    }
    return false;
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

    regfree(&rgx_temp_input);
    return false;
}

bool fan_control_enabled() {
    FILE* fp = fopen(FAN_PATH, "r");
    if (fp != NULL) {
        fclose(fp);
        return true;
    }
    fprintf(
        stderr,
        "error: thinkpad_acpi fan_control option disabled. Cannot continue!\n");
    return false;
}
