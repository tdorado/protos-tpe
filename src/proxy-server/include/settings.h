#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define DELIM "."
#define TEXT_BLOCK 100

struct settings{
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
    bool transformations;
    bool pipe_lining_supported;
    char *capa_text;
};

typedef struct settings * settings_t;

void print_usage();
void print_version();
void init_settings(settings_t settings);
bool valid_digit(char * digit);
bool valid_address(char * address);
bool valid_port(char * port);
bool valid_media_type(char * media_type);
bool valid_error_file(char * error_file);
int validate_and_set_settings(const int argc, char ** argv, settings_t settings);
int input_parser(int argc, char ** argv, settings_t settings);

#endif
