#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include <stdbool.h>

#define CMD_BUFFER 1024

#define DEFAULT_CMD "cat"
#define DEFAULT_ERROR_FILE "/dev/null"
#define DEFAULT_LOCAL_ADDR "0.0.0.0"
#define DEFAULT_LOCAL_PORT 1110
#define DEFAULT_MANAGEMENT_ADDR "127.0.0.1"
#define DEFAULT_MANAGEMENT_PORT 9090
#define DEFAULT_MEDIA_TYPES ""
#define DEFAULT_SERVER_PORT 110
#define DEFAULT_REPLACE_MESSAGE "Parte reemplazada."
#define POP3_FILTER_VERSION "1.0.0"

typedef struct settings * settings_t;

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
};

settings_t init_settings();
void print_usage();
void print_version();
bool valid_digit(char * digit);
bool valid_address(char * address);
bool valid_port(char * port);
bool valid_media_type(char * media_type);
bool valid_error_file(char * error_file);
bool valid_executable(char * command);
int validate_and_set_settings(const int argc, char ** argv, settings_t settings);
int input_parser(int argc, char ** argv, settings_t settings);
void free_settings(settings_t settings);

#endif