#include "args.h"

#include <getopt.h>

op_mode_t parse_args(int argc, char** argv) {
    switch (getopt(argc, argv, "hvr")) {
        case 'h':
            return OP_HELP;
        case 'v':
            return OP_VERSION;
        case 'r':
            return OP_RUN;
        default:
            return OP_UNKNOWN;
    }
}
