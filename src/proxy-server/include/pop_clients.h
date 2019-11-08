#ifndef POP_CLIENTS_H
#define POP_CLIENTS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <netdb.h>

#include "settings.h"
#include "metrics.h"
#include "constants.h"
#include "../../utils/include/buffer.h"
#include "../../utils/include/utils.h"

enum origin_server_states{
    ERROR_ORIGIN_SERVER = -1,
    RESOLVING_ORIGIN_SERVER,
    NOT_RESOLVED,
    CONNECTED_TO_ORIGIN_SERVER
};

enum client_states{
    CONNECTED,
    USER_REQUEST,
    USER_OK,
    PASS_REQUEST,
    PASS_OK,
    OTHER_REQUEST,
    OTHER_REQUEST_OK,
    RETR_REQUEST,
    RETR_OK,
    REMOVING_LAST_LINE,
    RETR_TRANSFORMING,
    RETR_FINISHED_TRANSFORMING
};

enum pop_reponses{
    OK_RESPONSE,
    ERR_RESPONSE
};

enum external_transformation_states{
    ERROR_TRANSFORMATION = -1,
    PROCESS_NOT_INITIALIZED,
    PROCESS_INITIALIZED,
    REMOVING_CRLF_DOT_CRLF,
    WAITING_TRANSFORMATION
};

typedef struct client * client_t;

struct client{

    int client_state;
    int client_fd;
    buffer_t client_read_buffer;
    buffer_t client_write_buffer;
    bool logged;

    int origin_server_state;
    int origin_server_fd;
    struct sockaddr_in6 origin_server_addr;
    buffer_t origin_server_read_buffer;
    buffer_t origin_server_write_buffer;
    bool received_greeting;

    int external_transformation_state;
    int external_transformation_read_fd;
    int external_transformation_write_fd;
    buffer_t transformation_buffer;

    client_t prev;
    client_t next;
};

struct client_list{
    client_t first;
    client_t last;
    int qty;
};

typedef struct client_list * client_list_t;

client_list_t init_client_list();
client_t create_client(client_list_t client_list, const int fd);
void remove_client(client_list_t client_list, client_t client);
void free_client_list(client_list_t client_list);
void accept_new_client(client_list_t client_list, const int server_fd, struct sockaddr_in6 server_addr, socklen_t server_addr_len, settings_t settings, metrics_t metrics);
int check_client_fds(client_t client, client_list_t client_list, int *max_fd, fd_set *read_fds, fd_set *write_fds, settings_t settings, metrics_t metrics);
void resolve_client(client_t client, client_list_t client_list, fd_set *read_fds, fd_set *write_fds, settings_t settings, metrics_t metrics);
void interpret_request(client_t client);
int interpret_response(buffer_t buff);
int check_external_transformation_fds(client_list_t client_list, client_t client, settings_t settings, fd_set *read_fds, fd_set *write_fds, metrics_t metrics);
void move_response_line(buffer_t from, buffer_t to);
int remove_crlf_dot_crlf(buffer_t buffer);

#endif