#ifndef PROXY_SOCKET_H
#define PROXY_SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>

#include "settings.h"
#include "metrics.h"
#include "proxy_clients.h"

int init_proxy_socket(struct sockaddr_in6 * server_addr, socklen_t * server_addr_len, settings_t settings);
void set_proxy_fd(const int proxy_fd, int * max_fd, fd_set * read_fds);
void resolve_proxy_client(int proxy_fd, fd_set * read_fds, client_list_t client_list, struct sockaddr_in6 server_addr, socklen_t * server_addr_len, settings_t settings, metrics_t metrics);

#endif
