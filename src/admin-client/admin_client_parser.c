#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "include/admin_client_parser.h"

bool valid_digit(char * digit);
bool valid_address(char * address);
bool valid_port(char * port);
void print_usage(void);

static char *login_token = "ZXN0YUFw";

bool valid_digit(char * digit) {
    while (*digit) {
        if (*digit >= '0' && *digit <= '9') {
            digit++;
        } else {
            return false;
        }
    }
    return true;
}

bool valid_address(char * address) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, address, &(sa.sin_addr)) == 1;
}

bool valid_port(char * port) {
    int num;

    if (!valid_digit(port)) {
        return false;
    }

    num = atoi(port);

    if (num >= 0 && num <= 65535) {
        return true;
    }

    return false;
}

void print_usage(void) {
    printf("USAGE: ./adminclient [POSIX style options]\n"
            "POSIX style options: \n"
            "   -p <manegement-port>        Specifies the port where the Manegement server is located. By default is 9090. \n"
            "   -a <manegement-address>     Specifies the address where the the Manegement server is located. By default is loopback. \n");
}

bool validate_and_set_params(const int argc, char ** argv, struct addr_and_port * addr_port) {
    if (argc == 1 || argc == 3 || argc == 5) {

    int c;
    bool error = false;

    strcpy(addr_port->addr, "127.0.0.1");
    addr_port->port = 9090;

    while ((c = getopt(argc, argv, "a:p:")) != EOF && !error) {
        switch (c) {
            case 'a':
                if (valid_address(optarg)){
                    strcpy(addr_port->addr, optarg);
                } else {
                    perror("Invalid -a <manegement-addr> argument. \n");
                    error = true;
                }
                break;
            case 'p':
                if (valid_port(optarg)){
                    addr_port->port = (uint16_t) atoi(optarg);
                } else {
                    perror("Invalid -p <manegement-port> argument. \n");
                    error = true;
                }
                break;
            default:
                break;
        }
    }

    return error;
    } else {
        print_usage();
        return true;
    }
}

bool parse_greeting(char msg_received[BUFFER_MAX], int received_len) {
    if(received_len == 1 && msg_received[0] == OK) {
        printf("+OK Connection established\n");
        return false;
    } else {
        printf("-ERR Closing\n");
        return true;
    }
}

void parse_on_buffer(char msg_to_send[BUFFER_MAX], int * to_send_len, char command) {
    msg_to_send[0] = command;
    *to_send_len = 1;
}

void printf_help(void) {
    printf("Commands:\n");
    printf("\tlogin\n");
    printf("\tlogout\n");
    printf("\tget cc(concurrent connections)\n");
    printf("\tget tb(total connections\n");
    printf("\tget bt(bytes transferidos)\n");
    printf("\tget mtypes\n");
    printf("\tget cmd\n");
    printf("\tset cmd <cmd>\n");
    printf("\tset mtypes <mtype,mtype,mtype,mtype...>\n");
    printf("\tenable mtype transformations\n");
    printf("\tenable cmd transformations\n");
    printf("\tdisable mtype transformations\n");
}

state_t parse_command(char msg_received[BUFFER_MAX], char msg_to_send[BUFFER_MAX], int * to_send_len) {
    if(strncasecmp(LOGIN_TEXT, msg_received, 5) == 0){
        parse_on_buffer(msg_to_send, to_send_len, LOGIN);
        strcpy(msg_to_send + 1, login_token);
        *to_send_len += 8;
        return LOGIN;
    } else if(strncasecmp(LOGOUT_TEXT, msg_received, 6) == 0) {
        parse_on_buffer(msg_to_send, to_send_len, LOGOUT);
        return LOGOUT;
    } else if(strncasecmp(GET_CONC_CONN_TEXT, msg_received, 6) == 0) {
        parse_on_buffer(msg_to_send, to_send_len, GET_CONCURRENT_CONNECTIONS);
        return GET_CONCURRENT_CONNECTIONS;
    } else if(strncasecmp(GET_TOTAL_CONN_TEXT, msg_received, 6) == 0) {
        parse_on_buffer(msg_to_send, to_send_len, GET_TOTAL_CONNECTIONS);
        return GET_TOTAL_CONNECTIONS;
    } else if(strncasecmp(GET_BYTES_TRANSF_TEXT, msg_received, 6) == 0) {
        parse_on_buffer(msg_to_send, to_send_len, GET_BYTES_TRANSFERED);
        return GET_BYTES_TRANSFERED;
    } else if(strncasecmp(GET_MTYPES_TEXT, msg_received, 10) == 0) {
        parse_on_buffer(msg_to_send, to_send_len, GET_MTYPES);
        return GET_MTYPES;
    } else if(strncasecmp(GET_CMD_TEXT, msg_received, 7) == 0) {
        parse_on_buffer(msg_to_send, to_send_len, GET_CMD);
        return GET_CMD;
    } else if(strncasecmp(SET_CMD_TEXT, msg_received, 7) == 0) {
        parse_on_buffer(msg_to_send, to_send_len, SET_CMD);
        strcpy(msg_to_send + 1, msg_received + 7 + 1 );
        *to_send_len += strlen(msg_received + 7 + 1) + 1;
        return SET_CMD;
    } else if(strncasecmp(SET_MTYPES_TEXT, msg_received, 10) == 0) {
        parse_on_buffer(msg_to_send, to_send_len, SET_MTYPES);
        strcpy(msg_to_send + 1, msg_received + 10 + 1);
        *to_send_len += strlen(msg_received + 10 + 1) + 1;
        return SET_MTYPES;
    } else if(strncasecmp(ENABLE_MTYPE_TEXT, msg_received, 28) == 0) {
        parse_on_buffer(msg_to_send, to_send_len, ENABLE_MTYPE_TRANSFORMATIONS);
        return ENABLE_MTYPE_TRANSFORMATIONS;
    } else if(strncasecmp(ENABLE_CMD_TEXT, msg_received, 26) == 0) {
        parse_on_buffer(msg_to_send, to_send_len, ENABLE_CMD_TRANSFORMATIONS);
        return ENABLE_CMD_TRANSFORMATIONS;
    } else if(strncasecmp(DISABLE_MTYPE_TEXT, msg_received, 29) == 0) {
        parse_on_buffer(msg_to_send, to_send_len, DISABLE_MTYPE_TRANSFORMATIONS);
        return DISABLE_MTYPE_TRANSFORMATIONS;
    } else if(strncasecmp(DISABLE_CMD_TEXT, msg_received, 27) == 0) {
        parse_on_buffer(msg_to_send, to_send_len, DISABLE_CMD_TRANSFORMATIONS);
        return DISABLE_CMD_TRANSFORMATIONS;
    } else if(strncasecmp(HELP_TEXT, msg_received, 4) == 0) {
        printf_help();
        return HELP;
    } else {
        return INVALID;
    }
}

bool print_first_response(char a) {
    if(a == OK){
        printf("+OK\n");
        return true;
    } else {
        printf("-ERR\n");
        return false;
    }
}

bool interpret_response(state_t state, char msg_received[BUFFER_MAX]) {
    bool ret = false;
    switch(state){
        case LOGIN:
            if(msg_received[0] == OK) {
                printf("+OK Logged in.\n");
            } else {
                printf("-ERR Incorrect.\n");
            }
            break;
        case LOGOUT:
            if(msg_received[0] == OK) {
                printf("+OK Logged out.\n");
                ret = true;
            } else {
                printf("-ERR.\n");
            }
            break;
        case GET_CONCURRENT_CONNECTIONS:
            if(msg_received[0] == OK) {
                printf("+OK ");
                printf("%s\n", msg_received + 1);
            } else {
                printf("-ERR\n");
            }
            break;
        case GET_TOTAL_CONNECTIONS:
            if(msg_received[0] == OK) {
                printf("+OK ");
                printf("%s\n", msg_received + 1);
            } else {
                printf("-ERR\n");
            }
            break;
        case GET_BYTES_TRANSFERED:
            if(msg_received[0] == OK) {
                printf("+OK ");
                printf("%s\n", msg_received + 1);
            } else {
                printf("-ERR\n");
            }
            break;
        case GET_MTYPES:
            if(msg_received[0] == OK){
                printf("+OK ");
                printf("%s\n", msg_received + 1);
            } else {
                printf("-ERR\n");
            }
            break;
        case GET_CMD:
            if(msg_received[0] == OK) {
                printf("+OK ");
                printf("%s\n", msg_received + 1);
            } else {
                printf("-ERR\n");
            }
            break;
        case SET_CMD:
            if(msg_received[0] == OK) {
                printf("+OK command setted: ");
                printf("%s\n", msg_received + 1);
            } else {
                printf("-ERR\n");
            }
            break;
        case SET_MTYPES:
            if(msg_received[0] == OK) {
                printf("+OK media types added: ");
                printf("%s \n", msg_received + 1);
            } else {
                printf("-ERR\n");
            }
            break;
        case ENABLE_MTYPE_TRANSFORMATIONS:
            if(msg_received[0] == OK) {
                printf("+OK Media Type transformations enabled.\n");
            } else {
                printf("-ERR Media Type transformations already enabled\n");
            }
            break;
        case ENABLE_CMD_TRANSFORMATIONS:
            if(msg_received[0] == OK) {
                printf("+OK CMD transformations enabled.\n");
            } else {
                printf("-ERR CMD transformations already enabled\n");
            }
            break;
        case DISABLE_MTYPE_TRANSFORMATIONS:
            if(msg_received[0] == OK) {
                printf("+OK Media type transformations disabled\n");
            } else {
                printf("-ERR Media type transformations already disabled\n");
            }
            break;
        case DISABLE_CMD_TRANSFORMATIONS:
            if(msg_received[0] == OK) {
                printf("+OK CMD transformations disabled\n");
            } else {
                printf("-ERR CMD transformations already disabled\n");
            }
            break;
        case HELP:
        case INVALID:
        default:
            printf("-ERR\n");
            break;
    }
    return ret;
}
