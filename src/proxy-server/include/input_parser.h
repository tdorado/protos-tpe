#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

#define DELIM "."

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

void print_usage();
void print_version();
void initialize_values(input_t proxy_params);
bool valid_digit(char * digit);
bool valid_address(char * address);
bool valid_port(char * port);
int validate_and_set_params(const int argc, char ** argv, input_t proxy_params);
int input_parser(int argc, char ** argv, input_t proxy_params);

#endif
