#include "util.h"

#include <dirent.h>
#include <regex.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "fan.h"

char *join_path(char *path, char *append) {
    if (path == NULL || append == NULL) {
        DBG("pointers invalid");
        return NULL;
    }
    int psize = strlen(path);
    int asize = strlen(append);
    int size = psize + asize + 2;
    char *buf = (char *)calloc(1, size * sizeof(char));
    if (buf == NULL) {
        fprintf(stderr, "Could not allocate buffer of size: %d\n", size);
        exit(0);
    }
    memcpy(buf, path, strlen(path));
    memcpy(buf + psize, "/", 1);
    memcpy(buf + psize + 1, append, asize);
    buf[size - 1] = '\0';
    return buf;
}

void add_to_list(char ***lst, uint16_t *lst_size, char *str) {
    if (lst == NULL || lst_size == NULL) {
        DBG("called with nil ptrs");
        return;
    }
    uint16_t first_empty = *lst_size;
    for (uint16_t i = 0; i < *lst_size; i++) {
        if ((*lst)[i] == NULL) {
            first_empty = i;
            break;
        }
    }
    if (first_empty == *lst_size) {
        *lst = realloc(*lst, ((*lst_size << 1) < UINT16_MAX) ? *lst_size << 1
                                                             : UINT16_MAX);
        for (uint16_t i = first_empty; i < *lst_size; i++) {
            (*lst)[i] = NULL;
        }
    }
    (*lst)[first_empty] = str;
}

void traverse_dir(char *path, regex_t patt, char **outlst, uint16_t *lst_size) {
    struct dirent *de;
    DIR *dr = opendir(path);
    if (dr == NULL) {
        DBGF("failed to open path: %s\n", path);
        return;
    }
    while ((de = readdir(dr)) != NULL) {
        if (strncmp(de->d_name, ".", 1) == 0 ||
            strncmp(de->d_name, "..", 2) == 0) {
            continue;
        }
        char *fpath = join_path(path, de->d_name);
        struct stat st;
        if (lstat(fpath, &st)) {
            printf("Could not stat path: %s\n", fpath);
            continue;
        }
        if (S_ISDIR(st.st_mode) && !S_ISLNK(st.st_mode)) {
            char *npath = join_path(path, de->d_name);
            traverse_dir(npath, patt, outlst, lst_size);
            free(npath);
        }
        if (S_ISREG(st.st_mode)) {
            if (regexec(&patt, de->d_name, 0, NULL, 0) == 0) {
                char *target = join_path(path, de->d_name);
                add_to_list(&outlst, lst_size, target);
            }
        }
        free(fpath);
    }
    closedir(dr);
}

void cleanup() { fan_cleanup(); }

void die(char *msg, int exit_code) {
    cleanup();
    if (system("echo level auto > " FAN_PATH) != 256) {
        fprintf(stderr, "%s\nfan level set to auto\n", msg);
    } else {
        fprintf(stderr, "%s\ncould not set fan level\n", msg);
        exit_code = EXIT_FAILURE;
    }
    exit(exit_code);
}

