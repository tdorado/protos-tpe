#ifndef POP_PARSER_H
#define POP_PARSER_H

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#include "buffer.h"
#include "settings.h"

typedef struct client_parser_state * client_parser_state_t;

struct client_parser_state {
    char command[4];
    int command_len;
    char username[80];
    int username_len;
    bool found_command;
    bool waiting_username;
};

typedef struct transformation_parser_state * transformation_parser_state_t;

struct out_transformation_parser_state {
    bool first_r_found;
    bool first_n_found;
    bool dot_found;
    bool second_r_found;
};

struct in_transformation_parser_state {
    buffer_t buffer;
    bool r_found;
    bool n_found;
    char last_char;
};

struct transformation_parser_state {
    struct out_transformation_parser_state out_ps;
    struct in_transformation_parser_state in_ps;
};

transformation_parser_state_t init_transformation_parser_state(void);
void reset_transformation_parser_state(transformation_parser_state_t transformation_parser_state);
ssize_t read_and_parse_transformation_from_fd(int fd, buffer_t buffer, transformation_parser_state_t transformation_parser_state);
ssize_t write_and_parse_transformation_to_fd(int fd, buffer_t buffer, transformation_parser_state_t transformation_parser_state);
void free_transformation_parser_state(transformation_parser_state_t transformation_parser_state);
client_parser_state_t init_client_parser_state(void);
void reset_client_parser_state(client_parser_state_t client_parser_state);
void free_client_parser_state (client_parser_state_t client_parser_state);

#endif
