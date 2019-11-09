#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>

#include "include/proxy_socket.h"

int init_proxy_socket(struct sockaddr_in6 *server_addr, socklen_t *server_addr_len, settings_t settings) {
    int proxy_fd = -1, flags = -1;

    memset(server_addr, 0, sizeof(*server_addr));

    if ((proxy_fd = socket(AF_INET6, SOCK_STREAM, 0)) == -1){
        perror("Error creating server socket");
        exit(EXIT_FAILURE);
    }

    flags = fcntl(proxy_fd, F_GETFL);

    if (fcntl(proxy_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("Error setting flags on server socket");
        exit(EXIT_FAILURE);
    }

    setsockopt(proxy_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    server_addr->sin6_family = AF_INET6;

    if (strcmp(settings->local_addr, "loopback") == 0){
        server_addr->sin6_addr = in6addr_loopback;
    }
    else if (strcmp(settings->local_addr, "any") == 0){
        server_addr->sin6_addr = in6addr_any;
    }
    else{
        inet_pton(AF_INET6, settings->local_addr, &(server_addr->sin6_addr));
    }

    server_addr->sin6_port = htons(settings->local_port);

    if (bind(proxy_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0){
        perror("Error binding server socket");
        exit(EXIT_FAILURE);
    }

    if (listen(proxy_fd, 10) < 0) {
        perror("Error listening server socket");
        exit(EXIT_FAILURE);
    }

    *server_addr_len = sizeof(*server_addr);

    return proxy_fd;
}

void set_proxy_fd(const int proxy_fd, int *max_fd, fd_set *read_fds){
    FD_SET(proxy_fd, read_fds);
    if (proxy_fd > *max_fd) {
        *max_fd = proxy_fd;
    }
}

void resolve_proxy_client(int proxy_fd, fd_set *read_fds, client_list_t client_list, struct sockaddr_in6 server_addr, socklen_t * server_addr_len, settings_t settings, metrics_t metrics) {
    if (FD_ISSET(proxy_fd, read_fds)) {
        accept_new_client(client_list, proxy_fd, server_addr, server_addr_len, settings, metrics);
    }
}