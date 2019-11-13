#ifndef POP_PARSER_H
#define POP_PARSER_H

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#include "buffer.h"

typedef struct parser_state * parser_state_t;

struct out_parser_state {
    bool first_r_found;
    bool first_n_found;
    bool dot_found;
    bool second_r_found;
};

struct in_parser_state {
    buffer_t buffer;
    bool r_found;
    bool n_found;
    char last_char;
};

struct parser_state {
    struct out_parser_state out_ps;
    struct in_parser_state in_ps;
};

parser_state_t init_parser_state(void);
void reset_parser_state(parser_state_t parser_state);
ssize_t read_and_parse_from_fd(int fd, buffer_t buffer, parser_state_t parser_state);
ssize_t write_and_parse_to_fd(int fd, buffer_t buffer, parser_state_t parser_state);
void free_parser_state(parser_state_t parser_state);

#endif
