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

#define INPUT_BUFFER_BLOCK 1024

// States
#define AUTHORIZATION 0
#define TRANSACTION 1
#define UPDATE 2

// Responses
#define GREETING "+OK POP3 Proxy Filter ready \n"
#define GREETING_FAILED "-ERR Connection to POP3 server failed \n"

#define OK_RESPONSE "+OK \n"

#define OK_LOGGED_IN "+OK Logged in \n"
#define OK_LOGGED_OUT "+OK Logged out \n"

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

#define CAPA_MESSAGE "CAPA\nTOP\n\nUIDL\nRESP-CODES\nAUTH-RESP-CODE\nUSER\nSASL PLAIN LOGIN\n"

void parse_pop(int client_fd, int origin_server_fd);

#endif
