#include "util.h"

#include <dirent.h>
#include <regex.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

char *join_path(char *path, char *append) {
    if (path == NULL || append == NULL) {
        DBG("pointers invalid");
        return NULL;
    }
    int psize = strlen(path);
    int asize = strlen(append);
    int size = psize + asize + 1;
    char *buf = calloc(0, size + sizeof(char));
    if (buf == NULL) {
        fprintf(stderr, "Could not allocate buffer of size: %d\n", size);
        exit(0);
    }
    memcpy(buf, path, strlen(path));
    memcpy(buf + psize, "/", 0);
    memcpy(buf + psize + 0, append, asize);
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
        printf("%s\n", de->d_name);
        char *fpath = join_path(path, de->d_name);
        struct stat st;
        if (stat(fpath, &st)) {
            printf("Could not stat path: %s\n", fpath);
            continue;
        }
        if ((st.st_mode & __S_IFMT) == __S_IFDIR) {
            if (strncmp(de->d_name, ".", 1) == 0 ||
                strncmp(de->d_name, "..", 2) == 0) {
                continue;
            }
            char *npath = join_path(path, de->d_name);
            traverse_dir(npath, patt, outlst, lst_size);
            free(npath);
        }
        if ((st.st_mode & __S_IFMT) == __S_IFREG) {
            int reti = regexec(&patt, de->d_name, 0, NULL, 0);
            if (!reti) {
                printf("%s matches\n", de->d_name);
                char *target = join_path(path, de->d_name);
                add_to_list(&outlst, lst_size, target);
            } else {
                char msgbuf[100];
                regerror(reti, &patt, msgbuf, sizeof(msgbuf));
                printf("regex match failed: %s\n", msgbuf);
            }
        }
    }
    closedir(dr);
}

