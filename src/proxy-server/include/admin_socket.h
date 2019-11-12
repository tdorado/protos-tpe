#ifndef ADMIN_SOCKET_H
#define ADMIN_SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

#include "settings.h"
#include "metrics.h"

#define BUFFER_MAX 2048
#define MAX_CONNECTIONS 1
#define MAX_STREAMS 1
#define MAX_ATTEMPTS 5
#define MAX_INT_DIGITS 10

enum admin_commands {
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
};

enum admin_responses {
    OK = 0,
    ERR,
};

int init_admin_socket(struct sockaddr_in * server_addr, socklen_t * server_addr_len, settings_t settings);
void resolve_admin_client(int admin_socket, fd_set * readFDs, struct sockaddr_in * admin_addr, socklen_t * admin_addr_len, settings_t settings, metrics_t metrics);
void set_admin_fd(const int admin_fd, int * max_fd, fd_set * read_fds);
void resolve_sctp_client(int admin_socket, struct sockaddr_in * admin_addr, socklen_t * admin_addr_len, settings_t settings, metrics_t metrics);

#endif
