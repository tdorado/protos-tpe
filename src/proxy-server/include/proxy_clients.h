#ifndef PROXY_CLIENTS_H
#define PROXY_CLIENTS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <netdb.h>

#include "settings.h"
#include "metrics.h"
#include "buffer.h"
#include "utils.h"
#include "pop_parser.h"

#define BUFFER_SIZE 2048

#define CRLF_DOT_CRLF "\r\n.\r\n"
#define CRLF_DOT_CRLF_LEN 5
#define OK_WELCOME "+OK Proxy filter ready\r\n"
#define OK_WELCOME_LEN 24
#define ERR_TRANSFORMATION "-ERR Transformation refused\r\n"
#define ERR_TRANSFORMATION_LEN 29
#define ERR_ORIGIN_SERVER_CONNECTION "-ERR Origin server connection refused\r\n"
#define ERR_ORIGIN_SERVER_CONNECTION_LEN 39

typedef enum origin_server_states {
    ERROR_ORIGIN_SERVER = -1,
    RESOLVING_ORIGIN_SERVER,
    NOT_RESOLVED_ORIGIN_SERVER,
    RESOLVED_TO_ORIGIN_SERVER
} origin_server_state_t;

typedef enum client_states {
    NOT_LOGGED_IN,
    PASS_REQUEST,
    LOGGED_IN,
    CAPA_REQUEST,
    CAPA_OK,
    RETR_REQUEST,
    RETR_OK,
    RETR_TRANSFORMING,
} client_state_t;

typedef enum pop_reponses {
    ERR_RESPONSE,
    OK_RESPONSE
} pop_response_t;

typedef enum external_transformation_states {
    ERROR_TRANSFORMATION_PROCESS = -1,
    PROCESS_NOT_INITIALIZED,
    PROCESS_INITIALIZED,
} external_transformation_state_t;

typedef struct client * client_t;

struct client {
    client_state_t client_state;
    int client_fd;
    buffer_t client_read_buffer;
    buffer_t client_write_buffer;
    bool logged;
    char command_received[4];
    size_t command_received_len;

    origin_server_state_t origin_server_state;
    int origin_server_fd;
    buffer_t origin_server_buffer;
    bool received_greeting;

    external_transformation_state_t external_transformation_state;
    int external_transformation_read_fd;
    int external_transformation_write_fd;
    parser_state_t parser_state;

    client_t prev;
    client_t next;
};

struct client_list {
    client_t first;
    client_t last;
    int qty;
};

typedef struct client_list * client_list_t;

client_list_t init_client_list(void);
client_t create_client(client_list_t client_list, const int fd);
void remove_client(client_list_t client_list, client_t client);
void free_client_list(client_list_t client_list);
void add_client(client_list_t client_list, const int proxy_fd, struct sockaddr_in6 server_addr, socklen_t * server_addr_len, settings_t settings, metrics_t metrics);
int set_client_fds(client_t client, client_list_t client_list, int * max_fd, fd_set * read_fds, fd_set * write_fds, settings_t settings, metrics_t metrics);
void set_client_fd(client_t client, fd_set * read_fds, fd_set * write_fds);
int set_origin_server_fd(client_list_t client_list, fd_set * read_fds, fd_set * write_fds, client_t client, settings_t settings, metrics_t metrics);
int set_external_transformation_fds(client_list_t client_list, client_t client, settings_t settings, fd_set * read_fds, fd_set * write_fds, metrics_t metrics);
void resolve_client(client_t client, client_list_t client_list, fd_set * read_fds, fd_set * write_fds, settings_t settings, metrics_t metrics);
pop_response_t get_response(buffer_t buffer);

#endif
