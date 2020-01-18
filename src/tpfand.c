#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

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

void cleanup() { fan_cleanup(); }

void die(char* msg, int exit_code) {
    cleanup();
    if (system("echo level auto > " FAN_PATH) != 256) {
        fprintf(stderr, "%s\nfan level set to auto\n", msg);
    } else {
        fprintf(stderr, "%s\ncould not set fan level\n", msg);
        exit_code = EXIT_FAILURE;
    }
    exit(exit_code);
}

void signal_handler(int sig) {
    char msg[32];
    snprintf(msg, sizeof(msg), "caught signal %d\n", sig);
    die(msg, EXIT_SUCCESS);
}

void run() {
#ifdef _GNU_SOURCE
    static struct sigaction sig_handler;
    memset(&sig_handler, 0, sizeof(sig_handler));
    sig_handler.sa_handler = signal_handler;
    sigemptyset(&sig_handler.sa_mask);
    sigaction(SIGINT, &sig_handler, NULL);
    sigaction(SIGTERM, &sig_handler, NULL);
#endif

    /*
    if (!fan_control_enabled()) {
        die("", EXIT_FAILURE);
    }
    */

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

    while (1) {
        DBG("going to sleep");
        sleep(cfg.poll_inter);
        DBG("waking up");
    }
}
