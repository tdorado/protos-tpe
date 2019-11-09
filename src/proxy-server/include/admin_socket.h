#ifndef ADMIN_SOCKET_H
#define ADMIN_SOCKET_H

#include <sys/socket.h>
#include <netinet/in.h>

#include "settings.h"
#include "metrics.h"
#include "proxy_clients.h"

void set_admin_fd(const int admin_fd, int *max_fd, fd_set *read_fds);
void resolve_admin_client(int adminSocket, fd_set *readFDs, struct sockaddr_in *adminAddress, socklen_t *adminAddressLength, settings_t *st, metrics_t *mtx);
int init_admin_socket(struct sockaddr_in *admin_addr, socklen_t *admin_addr_len, settings_t settings);

#endif