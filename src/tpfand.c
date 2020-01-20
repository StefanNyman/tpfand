#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "args.h"
#include "config.h"
#include "fan.h"
#include "util.h"

void help();
void version();
void run();

int main(int argc, char** argv) {
    switch (parse_args(argc, argv)) {
        case OP_HELP:
            help();
            break;
        case OP_VERSION:
            version();
            break;
        case OP_RUN:
            run();
            break;
        case OP_UNKNOWN:
            printf("Invalid arguments\n");
            help();
            break;
    }
    return 0;
}

void help() {
    version();
    printf("\n\t -h: display this help\n");
    printf("\t -v: display version\n");
    printf("\t -r: run\n\n");
}

void version() {
#if defined(BINARY) && defined(VERSION)
    printf("%s: %s\n", BINARY, VERSION);
#endif
}

void signal_handler(int sig) {
    char msg[32];
    snprintf(msg, sizeof(msg), "caught signal %d\n", sig);
    die(msg, EXIT_SUCCESS);
}

#define FOR_EVER for (;;)

void run() {
#ifdef _GNU_SOURCE
    static struct sigaction sig_handler;
    memset(&sig_handler, 0, sizeof(sig_handler));
    sig_handler.sa_handler = signal_handler;
    sigemptyset(&sig_handler.sa_mask);
    sigaction(SIGINT, &sig_handler, NULL);
    sigaction(SIGTERM, &sig_handler, NULL);
#endif

    if (!fan_control_enabled()) {
        die("", EXIT_FAILURE);
    }

    if (!find_max_temp_path()) {
        die("", EXIT_FAILURE);
    }

    if (!find_input_temp_path()) {
        die("", EXIT_FAILURE);
    }

#if defined(BINARY) && defined(VERSION)
    printf("%s: %s starting\n", BINARY, VERSION);
#endif
    config_t cfg;
    default_config(&cfg);
    read_config(&cfg);

    cfg.max_temp = get_max_temp();
    uint8_t curr_temp = get_curr_temp();
    tmp_direction_t dir = INC;
    uint8_t curr_level = INVALID_LEVEL;

    FOR_EVER {
        uint8_t prev_temp = curr_temp;
        curr_temp = get_curr_temp();
        uint8_t prev_level = curr_level;

        if (level_changed(curr_temp, prev_level, dir, &cfg)) {
            dir = ((curr_temp - prev_temp) > 0) ? INC : DEC;
            curr_level = compute_level(curr_temp, dir, &cfg);
            set_fan_level(curr_level);
            fprintf(stderr, "lvl: %d(%d) -> %d(%d)\n", prev_level, prev_temp,
                    curr_level, curr_temp);
        }
        sleep(cfg.poll_inter);
    }
}
