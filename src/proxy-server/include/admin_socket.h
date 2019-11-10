#ifndef ADMIN_SOCKET_H
#define ADMIN_SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

#include "settings.h"
#include "metrics.h"
#include "proxy_clients.h"

typedef enum requestType {
    LOGIN_REQUEST = 0x01,
    LOGOUT_REQUEST, 
    GET_REQUEST, 
    SET_REQUEST,
    RM_REQUEST
} requestType;

typedef enum operations {
    CONCURRENT = 0X01,
    ACCESSES,
    BYTES,
    MTYPES,
    CMD
} operations;

typedef enum response_type { // is equivalent to request type, it kinda be duplicated code, but we win on expresiveness.
    LOGIN_RESPONSE = 0x01,
    LOGOUT_RESPONSE,
    GET_RESPONSE,
    SET_RESPONSE,
    RM_RESPONSE
} response_type;

int init_admin_socket(struct sockaddr_in *server_addr, socklen_t *server_addr_len, settings_t settings);
void resolve_admin_client(int admin_socket, fd_set *readFDs, struct sockaddr_in *admin_addr, socklen_t *admin_addr_len, settings_t settings, metrics_t metrics);
void set_admin_fd(const int admin_fd, int *max_fd, fd_set *read_fds);
void resolve_sctp_client(int admin_socket, struct sockaddr_in *admin_addr, int admin_addr_len, settings_t settings, metrics_t metrics);
int start_listen(int fd, int backlog);
int set_socket_opt(int admin_socket, int level, int opt_name, void *opt_val, socklen_t opt_len);
int binding(int admin_socket, struct sockaddr_in *server_addr, size_t server_addr_size);
int create_sctp_socket();

// parsing
bool parseLoginOrLogout(__uint8_t *admin_received_msg);
bool isTokenValid(__uint8_t *token);
char *parse_request(__uint8_t *admin_received_msg, settings_t settings, metrics_t metrics);
char *parse_get(__uint8_t *admin_received_msg, settings_t settings, metrics_t metrics);
char *buildMetricsResponse(int data, char *initial_msg);
char *buildGetResponseWithString(char *data, char *initial_msg);
char *parse_set(__uint8_t *admin_received_msg, settings_t settings, metrics_t metrics);
char *parse_rm(__uint8_t *admin_received_msg, settings_t settings, metrics_t metrics);
char *filter_repetitions_mtypes(char *current_mtypes, char *new_mtypes);
char *rm_mtypes(char *current_mtypes, char *mtypes_to_rm);

#endif