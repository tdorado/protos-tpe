#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H

#include <stdint.h>

struct input_params{
    char *origin_server_addr;
    uint16_t origin_server_port;
    char *management_addr;
    uint16_t management_port;
    char *local_addr;
    uint16_t local_port;
    char *replace_message;
    char *media_types;
    char *cmd;
    char *error_file;
};

typedef struct input_params * input_t;

int input_parser(int argc, char ** argv, input_t proxy_params);

#endif