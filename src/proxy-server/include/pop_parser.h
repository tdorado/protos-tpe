#ifndef POP_PARSER_H
#define POP_PARSER_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "settings.h"

#define TIMEOUT_NOT_LOGGED 180 //in seconds
#define TIMEOUT_LOGGED 600 //in seconds

#define INPUT_BUFFER_BLOCK 2048

// Responses
#define GREETING "+OK POP3 Proxy Filter ready \n"
#define GREETING_FAILED "-ERR Connection to POP3 server failed \n"

#define OKK_RESPONSE "+OK \n"

#define OK_LOGGED_IN "+OK Logged in \n"
#define OK_LOGGED_OUT "+OK Logging out. \n"

#define ERR_TIMEOUT_RESPONSE "-ERR timeout connection ended. \n"

#define USEROK "+OK %s is a valid mailbox \n"
#define USERERR "-ERR never heard of mailbox %s \n"

#define PASSOK "+OK maildrop locked and ready \n"
#define PASSINVALID "-ERR invalid password \n"
#define PASSMAILALREADYLOCK "-ERR unable to lock maildrop \n"

#define RETROK "+OK message follows \n"
#define RETRERR "-ERR no such message \n"

#define RSETOK "+OK \n"

#define DELEOK "+OK message deleted \n"
#define DELEERR "-ERR no such message \n"

#define NOOPOK "+OK \n"

#define STATOK "+OK nn mm \n" //nn mm are two int values

#define LISTOK "+OK scan listing follows \n"
#define LISTERR "-ERR no such message \n"

#define PARSERERR "ERROR WHILE PARSING COMANDS \n"

#define ERR_INVALID "-ERR Invalid command \n"
#define ERR_INVALID_TOO_MANY "-ERR Too many invalid commands \n"

#define CAPA_MESSAGE "CAPA\nTOP\nUIDL\nRESP-CODES\nAUTH-RESP-CODE\nUSER\nSASL PLAIN LOGIN\n"

int request_greeting(int origin_server_fd, char ** input_buffer, int *buffer_size);
void send_greeting(int client_fd);

void send_socket_message_from_buffer(int fd, char * input_buffer, int n);
void send_socket_message(int fd, char * message, int n);
int request_socket_message(int fd, char ** input_buffer, int * buffer_size);

void parse_pop(int client_fd, int origin_server_fd, settings_t settings);
int parse_capa_cmd(char * input, int n);
int parse_user_cmd(char * input, int n);
int parse_pass_cmd(char * input, int n);
int parse_quit_cmd(char * input, int n);
int parse_retr_or_rset_cmd(char * input, int n);
int parse_retr_cmd(char * input, int n);
int parse_rset_cmd(char * input, int n);
int parse_dele_cmd(char * input, int n);
int parse_noop_cmd(char * input, int n);
int parse_stat_cmd(char * input, int n);
int parse_list_cmd(char * input, int n);

#endif
