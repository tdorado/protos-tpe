#ifndef ADMIN_CLIENT_PARSER_H
#define ADMIN_CLIENT_PARSER_H

#include <stdbool.h>

struct addr_and_port {
    char addr[20];
    uint16_t port;
};

typedef enum admin_commands {
    HELP = -2,
    INVALID = -1,
    LOGIN = 0,
    LOGOUT,
    GET_CONCURRENT_CONNECTIONS,
    GET_TOTAL_CONNECTIONS,
    GET_BYTES_TRANSFERED,
    GET_MTYPES,
    GET_CMD,
    SET_CMD,
    SET_MTYPE,
    RM_MTYPE,
    ENABLE_MTYPE_TRANSFORMATIONS,
    ENABLE_CMD_TRANSFORMATIONS,
    DISABLE_TRANSFORMATIONS
} state_t;

enum admin_responses {
    OK = 0,
    ERR,
};

#define BUFFER_MAX 2048

#define HELP_TEXT "HELP"
#define LOGIN_TEXT "LOGIN"
#define LOGOUT_TEXT "LOGOUT"
#define GET_CONC_CONN_TEXT "GET CC"
#define GET_TOTAL_CONN_TEXT "GET TC"
#define GET_BYTES_TRANSF_TEXT "GET BT"
#define GET_MTYPES_TEXT "GET MTYPES"
#define GET_CMD_TEXT "GET CMD"
#define SET_CMD_TEXT "SET CMD"
#define SET_MTYPE_TEXT "SET MTYPE"
#define RM_MTYPE_TEXT "RM MTYPE"
#define ENABLE_MTYPE_TEXT "ENABLE MTYPE TRANSFORMATIONS"
#define ENABLE_CMD_TEXT "ENABLE CMD TRANSFORMATIONS"
#define DISABLE_TRANSF_TEXT "DISABLE TRANSFORMATIONS"

bool validate_and_set_params(const int argc, char ** argv, struct addr_and_port *addr_port);
bool parse_greeting(char msg_received[BUFFER_MAX], int received_len);
void parse_on_buffer(char msg_to_send[BUFFER_MAX], int * to_send_len, char command);
void printf_help();
state_t parse_command(char msg_received[BUFFER_MAX], char msg_to_send[BUFFER_MAX], int * to_send_len);
bool print_first_response(char a);
bool interpret_response(state_t state, char msg_received[BUFFER_MAX]);

#endif