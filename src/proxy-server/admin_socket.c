#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/sctp.h>
#include <sys/select.h>

#include "include/admin_socket.h"
#include "include/admin_thread.h"
#include "include/admin_parser.h"
#include "include/logs.h"

int start_listen(int fd, int backlog);
int set_socket_opt(int admin_socket, int level, int opt_name, void * opt_val, socklen_t opt_len);
int binding(int admin_socket, struct sockaddr_in * server_addr, size_t server_addr_size);
int create_sctp_socket();

int create_sctp_socket() {
    int admin_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (admin_fd == -1) {
        perror("Error creating admin socket");
        exit(EXIT_FAILURE);
    }
    return admin_fd;
}

int binding(int admin_fd, struct sockaddr_in *admin_addr, size_t admin_addr_size) {
    int binding_fd = bind(admin_fd, (struct sockaddr *)admin_addr, admin_addr_size);
    if (binding_fd == -1) {
        perror("Error on bind admin");
        close(admin_fd);
        exit(EXIT_FAILURE);
    }
    return binding_fd;
}

int set_socket_opt(int admin_fd, int level, int opt_name, void *opt_val, socklen_t opt_len) {
    int set_socket_fd = setsockopt(admin_fd, level, opt_name, opt_val, opt_len);
    if (set_socket_fd == -1) {
        perror("Error on setsockopt admin");
        close(admin_fd);
        exit(EXIT_FAILURE);
    }
    return set_socket_fd;
}

int start_listen(int fd, int max_connections) {
    int listen_fd = listen(fd, max_connections);
    if (listen_fd == -1) {
        perror("Error on listen admin");
        close(fd);
        exit(EXIT_FAILURE);
    }
    return listen_fd;
}

int init_admin_socket(struct sockaddr_in *admin_addr, socklen_t *admin_addr_len, settings_t settings) {
    int admin_fd;
    struct sctp_initmsg init_msg;

    memset(admin_addr, 0, sizeof(*admin_addr));

    admin_fd = create_sctp_socket();

    admin_addr->sin_family = AF_INET;
    if (strcmp(settings->management_addr, "loopback") == 0) {
        admin_addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    } else if (strcmp(settings->management_addr, "any") == 0) {
        admin_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        inet_pton(AF_INET, settings->management_addr, &(admin_addr->sin_addr));
    }

    admin_addr->sin_port = htons(settings->management_port);

    binding(admin_fd, admin_addr, sizeof(*admin_addr));

    memset(&init_msg, 0, sizeof(init_msg));
    init_msg.sinit_num_ostreams = MAX_STREAMS;
    init_msg.sinit_max_instreams = MAX_STREAMS;
    init_msg.sinit_max_attempts = MAX_ATTEMPTS;
    set_socket_opt(admin_fd, IPPROTO_SCTP, SCTP_INITMSG, &init_msg, sizeof(init_msg));

    start_listen(admin_fd, MAX_CONNECTIONS);

    return admin_fd;
}

void resolve_admin_client(int admin_fd, fd_set * readFDs, struct sockaddr_in * admin_addr, socklen_t * admin_addr_len, settings_t settings, metrics_t metrics) {
    if (FD_ISSET(admin_fd, readFDs)) {
        resolve_admin_fd_in_thread(admin_fd, admin_addr, admin_addr_len, settings, metrics);
    }
}

void set_admin_fd(const int admin_fd, int * max_fd, fd_set * read_fds) {
    FD_SET(admin_fd, read_fds);
    if (admin_fd > *max_fd) {
        *max_fd = admin_fd;
    }
}

void resolve_sctp_client(int admin_fd, struct sockaddr_in *admin_addr, socklen_t * admin_addr_len, settings_t settings, metrics_t metrics) {
    int connection_fd = -1;
    int ret = -1;
    int msg_received_len = -1;
    char msg_received[BUFFER_MAX];
    int msg_response_len = -1;
    char msg_response[BUFFER_MAX];
    bool logged = false;
    bool stop = false;

    connection_fd = accept(admin_fd, (struct sockaddr *)admin_addr, admin_addr_len);
    if (connection_fd == -1) {
        perror("Error accepting admin");
        return;
    }
    parse_ok_response(msg_response, &msg_response_len);
    ret = sctp_sendmsg(connection_fd, (void *)msg_response, msg_response_len, NULL, 0, 0, 0, 0, 0, 0);
    if (ret == -1) {
        printf("Error in sctp_sendmsg()\n");
        perror("Error sending admin message");
        stop = true;
    } else {
        log_message(false, "Successfully sent data to admin");
    }
    while(!stop) {
        msg_received_len = sctp_recvmsg(connection_fd, msg_received, BUFFER_MAX, NULL, 0, 0, 0);
        if (msg_received_len == -1) {
            printf("Error in sctp_recvmsg()\n");
            perror("Error receiving admin message");
            break;
        } else {
            log_message(false, "Successfully received data from admin");
            stop = parse_msg_received(&logged, msg_received, msg_received_len, msg_response, &msg_response_len, settings, metrics);
        }

        if (msg_received_len != 0) {
            ret = sctp_sendmsg(connection_fd, (void *)msg_response, msg_response_len, NULL, 0, 0, 0, 0, 0, 0);
            if (ret == -1) {
                printf("Error in sctp_sendmsg()\n");
                perror("Error sending admin message");
                break;
            } else {
                log_message(false, "Successfully sent data to admin");
            }
        }
    }
    close(connection_fd);
}
