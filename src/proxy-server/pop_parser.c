#include "include/pop_parser.h"

void parse_pop(int client_fd, int origin_server_fd) {

    char **input_buffer = malloc(sizeof(char*));
    *input_buffer = malloc(INPUT_BUFFER_BLOCK);
    int *buffer_size = malloc(sizeof(int));
    *buffer_size = INPUT_BUFFER_BLOCK;
    char c;

    bool user_logged = false;
    bool error = false;
    bool exit = false;

    if(request_greeting(origin_server_fd, input_buffer, buffer_size)){
        send_greeting(client_fd);
    }
    else{
        send_socket_message(client_fd, GREETING_FAILED, strlen(GREETING_FAILED));
        exit = true;
    }

    int invalid_counter = 0;
    int n;
    int r;
    while(!exit) {
        error = false;
        n = request_socket_message(client_fd, input_buffer, buffer_size);
        switch (c = tolower(*input_buffer[0])) {
            case 'c': // CAPA
                if (parse_capa_cmd(*input_buffer, n)){
                    send_socket_message(client_fd, CAPA_MESSAGE, strlen(CAPA_MESSAGE));
                }
                else{
                    error = true;
                }
                break;
            case 'u': // as there is only one command startin with u char, then, it has to be USER cmd
                if (parse_user_cmd(*input_buffer, n)) {
                    send_socket_message(origin_server_fd, *input_buffer, n);
                    n = request_socket_message(origin_server_fd, input_buffer, buffer_size);
                    if(strncmp("+OK", *input_buffer, 3) == 0){
                        send_socket_message(client_fd, OK_RESPONSE, strlen(OK_RESPONSE));
                    }
                }
                else{
                    error = true;
                }
                break;
            case 'p': // as there is only one command startin with u char, then, it has to be USER cmd
                if (parse_pass_cmd(*input_buffer, n)){
                    send_socket_message(origin_server_fd, *input_buffer, n);
                    n = request_socket_message(origin_server_fd, input_buffer, buffer_size);
                    if(strncmp("+OK", *input_buffer, 3) == 0){
                        send_socket_message(client_fd, OK_LOGGED_IN, strlen(OK_LOGGED_IN));
                        user_logged = true;
                    }
                }
                else {
                    error = true;
                }
                break;
            case 'q':
                if(parse_quit_cmd(*input_buffer, n)){
                    send_socket_message(origin_server_fd, *input_buffer, n);
                    n = request_socket_message(origin_server_fd, input_buffer, buffer_size);
                    if(strncmp("+OK", *input_buffer, 3) == 0){
                        send_socket_message(client_fd, OK_LOGGED_OUT, strlen(OK_LOGGED_OUT));
                        exit = true;
                    }
                }
                else {
                    error = true;
                }
                break;
            case 's': //STAT
                if (user_logged && parse_stat_cmd(*input_buffer, n)){
                    send_socket_message(origin_server_fd, *input_buffer, n);
                    n = request_socket_message(origin_server_fd, input_buffer, buffer_size);
                    send_socket_message(client_fd, *input_buffer, n);
                }
                else{
                    error = true;
                }
                break;
            case 'l': //LIST [msg] es optional, refiere a un msg-number
                if (user_logged && parse_list_cmd(*input_buffer, n)){
                    send_socket_message(origin_server_fd, *input_buffer, n);
                    n = request_socket_message(origin_server_fd, input_buffer, buffer_size);
                    send_socket_message(client_fd, *input_buffer, n);
                }
                else {
                    error = true;
                }
                break;
            case 'r': //RETR msg o RSET
                if (user_logged && ((r = parse_retr_or_rset_cmd(*input_buffer, n)) == 1 || r == 2) ){
                    if( r == 1 ){ // RETR
                        send_socket_message(origin_server_fd, *input_buffer, n);
                        n = request_socket_message(origin_server_fd, input_buffer, buffer_size);
                        send_socket_message(client_fd, *input_buffer, n);
                    }
                    else{        // RSET
                        send_socket_message(origin_server_fd, *input_buffer, n);
                        n = request_socket_message(origin_server_fd, input_buffer, buffer_size);
                        send_socket_message(client_fd, *input_buffer, n);
                    }
                }
                else{
                    error = true;
                }
                break;
            case 'd': // DELE msg
                if (user_logged && parse_dele_cmd(*input_buffer, n) ){
                    send_socket_message(origin_server_fd, *input_buffer, n);
                    n = request_socket_message(origin_server_fd, input_buffer, buffer_size);
                    send_socket_message(client_fd, *input_buffer, n);
                }
                else {
                    error = true;
                }
                break;
            case 'n': //NOOP
                if (user_logged && parse_noop_cmd(*input_buffer, n)){
                    send_socket_message(origin_server_fd, *input_buffer, n);
                    n = request_socket_message(origin_server_fd, input_buffer, buffer_size);
                    send_socket_message(client_fd, *input_buffer, n);
                }
                else{
                    error = true;
                }
                break;
            default: // ERROR
                error = true;
                break;
        }
        if(error == true){
            send_socket_message(client_fd, ERR_INVALID, strlen(ERR_INVALID));
            invalid_counter++;
        }else{
            invalid_counter = 0;
        }
        if(user_logged){
            if (invalid_counter == 21){
                exit = true;
                send_socket_message(client_fd, ERR_INVALID_TOO_MANY, strlen(ERR_INVALID_TOO_MANY));
            }
        }
        else {
            if (invalid_counter == 3){
                exit = true;
                send_socket_message(client_fd, ERR_INVALID_TOO_MANY, strlen(ERR_INVALID_TOO_MANY));
            }
        }
    }
    free(*input_buffer);
    free(input_buffer);
    free(buffer_size);
}

int request_greeting(int origin_server_fd, char **input_buffer, int *buffer_size){
    request_socket_message(origin_server_fd, input_buffer, buffer_size);
    if(strncmp("+OK", *input_buffer, 3) == 0){
        return 1;
    }
    return 0;
}

void send_greeting(int client_fd){
    send_socket_message(client_fd, GREETING, strlen(GREETING));
}

void send_socket_message_from_buffer(int fd, char* input_buffer, int n){
    send_socket_message(fd, input_buffer, n);
}

void send_socket_message(int fd, char* message, int n){
    send(fd, message, n, 0);
}

int request_socket_message(int fd, char** input_buffer, int *buffer_size){
    int ret = 0;
    int n = 0;
    n = recv(fd, *input_buffer, INPUT_BUFFER_BLOCK, 0);
    while(n == *buffer_size){
        *buffer_size += INPUT_BUFFER_BLOCK;
        *input_buffer = realloc(*input_buffer, *buffer_size);
        ret += n;
        n = recv(fd, *input_buffer, INPUT_BUFFER_BLOCK, 0);
    }
    ret += n;
    return ret;
}

int parse_retr_or_rset_cmd(char *input, int n){
    if (((char) tolower(input[1])) == 'e') {
        if(parse_retr_cmd(input, n)){
            return 1;
        }
    } else if (((char) tolower(input[1])) == 's') {
        if(parse_rset_cmd(input, n)){
            return 2;
        }
    }
    return 0;
}

int parse_quit_cmd(char *input, int n) {
    if (!strncasecmp(input, "QUIT", 4) && n == 5) {
        return 1;
    }
    return 0;
}

int parse_user_cmd(char *input, int n) {
    if (!strncasecmp(input, "USER", 4) && n > 5) {
        return 1;
    }
    return 0;
}

int parse_pass_cmd(char* input, int n) {
    if (!strncasecmp(input, "PASS", 4) && n > 5) {
        return 1;
    }
    return 0;
}

int parse_retr_cmd(char *input, int n) {
    if (!strncasecmp(input, "RETR", 4) && n > 5) {
        return 1;
    }
    return 0;
}

int parse_rset_cmd(char *input, int n) {
    if (!strncasecmp(input, "RSET", 4) && n == 5) {
        return 1;
    }
    return 0;
}

int parse_dele_cmd(char *input, int n) {
    if (!strncasecmp(input, "DELE", 4) && n > 5) {
        return 1;
    }
    return 0;
}

int parse_noop_cmd(char *input, int n) {
    if (!strncasecmp(input, "NOOP", 4) && n == 5) {
        return 1;
    }
    return 0;
}

int parse_stat_cmd(char *input, int n) {
    if (!strncasecmp(input, "STAT", 4) && n == 5) {
        return 1;
    }
    return 0;
}

int parse_capa_cmd(char *input, int n){
    if (!strncasecmp(input, "CAPA", 4) && n == 5) {
        return 1;
    }
    return 0;
}

int parse_list_cmd(char *input, int n) {
    if (!strncasecmp(input, "LIST", 4)) {
        return 1;
    }
    return 0;
}
