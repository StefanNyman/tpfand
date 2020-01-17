#ifndef _ARGS_H
#define _ARGS_H 1

typedef enum op_mode { OP_HELP, OP_VERSION, OP_RUN, OP_UNKNOWN } op_mode_t;

op_mode_t parse_args(int argc, char** argv);

#endif
