#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <limits.h>
#include <net/if.h>
#include <netinet/sctp.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#include "include/pop_parser.h"
#include "include/settings.h"
#include "include/logs.h"
#include "include/metrics.h"
#include "include/pop_clients.h"
#include "include/error_file.h"

static settings_t settings;
static metrics_t metrics;
static client_list_t client_list;
static int server_fd;
//static int admin_fd;

int init_server_socket(struct sockaddr_in6 *server_addr, socklen_t *server_addr_len, settings_t settings) {
    int server_fd = -1, flags = -1;

    memset(server_addr, 0, sizeof(*server_addr));

    if ((server_fd = socket(AF_INET6, SOCK_STREAM, 0)) == -1){
        perror("Error creating server socket");
        exit(EXIT_FAILURE);
    }

    flags = fcntl(server_fd, F_GETFL);

    if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("Error setting flags on server socket");
        exit(EXIT_FAILURE);
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

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

    if (bind(server_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0){
        perror("Error binding server socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Error listening server socket");
        exit(EXIT_FAILURE);
    }

    *server_addr_len = sizeof(*server_addr);

    return server_fd;
}

void set_server_fd(const int *server_fd, int *max_fd, fd_set *read_fds){
    FD_SET(*server_fd, read_fds);
    if (*server_fd > *max_fd) {
        *max_fd = *server_fd;
    }
}

void resolve_proxy_client(int server_fd, fd_set *read_fds, client_list_t client_list, struct sockaddr_in6 server_addr, int server_addr_len, settings_t settings, metrics_t metrics) {
    if (FD_ISSET(server_fd, read_fds)) {
        accept_new_client(client_list, server_fd, server_addr, server_addr_len, settings, metrics);
    }
}


void resolve(client_list_t client_list, fd_set read_fds, fd_set write_fds, settings_t settings, metrics_t metrics) {
    client_t client = client_list->first;
    while (client != NULL) {
        resolve_client(client, client_list, &read_fds, &write_fds, settings, metrics);
        client = client->next;
    }
}

void turn_off(void) {
    printf("Turning off server.\n");
    free_client_list(client_list);
    free(settings);
    free_metrics(metrics);
    //close(admin_fd);
    close(server_fd);

    exit(EXIT_FAILURE);
}

void signal_action_handler(const int signal_number) {
    if (signal_number == SIGINT) {
        printf("SIGINT was catched!\n");
        turn_off();
    }
    else if (signal_number == SIGPIPE) {
        printf("SIGPIPE was catched!\n");
        perror("pipe");
        turn_off();
    }
    else if (signal_number == SIGTERM) {
        printf("SIGTERM was catched!\n");
        turn_off();
    }
    else if (signal_number == SIGSEGV) {
        printf("SIGSEGV was catched!\n");
        turn_off();
    }
}

int set_up_signals(void) {
    struct sigaction sa;
    sa.sa_handler = signal_action_handler;
    if (sigaction(SIGINT, &sa, 0) != 0) {
        perror("sigaction()");
        turn_off();
        return -1;
    }
    if (sigaction(SIGPIPE, &sa, 0) != 0) {
        perror("sigaction()");
        turn_off();
        return -1;
    }
    if (sigaction(SIGTERM, &sa, 0) != 0) {
        perror("sigaction()");
        turn_off();
        return -1;
    }
    if (sigaction(SIGSEGV, &sa, 0) != 0) {
        perror("sigaction()");
        turn_off();
        return -1;
    }
    return 0;
}

static void wakeUpHandler(int signal)
{
  printf("Finished resolving host.\n");
}

int main(int argc, char ** argv) {

    if (set_up_signals() != 0) {
        exit(EXIT_FAILURE);
    }

    settings = malloc(sizeof(*settings));
    
    if (input_parser(argc, argv, settings) < 0) {
        free(settings);
        exit(EXIT_FAILURE);
    }
    redirect_stderr(settings);

    metrics = init_metrics();
    client_list = init_client_list();

    int res;
    server_fd = 0;
    //admin_fd = 0;
    int max_fd = 0;
    fd_set read_fds, write_fds;
    struct sockaddr_in6 server_addr;
    //struct sockaddr_in admin_addr;
    socklen_t server_addr_len;
    //socklen_t admin_addr_len;

    //memset(&admin_addr, 0, sizeof(admin_addr));

    server_fd = init_server_socket(&server_addr, &server_addr_len, settings);
    //admin_fd = init_admin_socket(&admin_addr, &admin_addr_len, settings);


    struct sigaction signalAction = {
        .sa_handler = wakeUpHandler,
    };

    sigset_t emptyset, blockset;
    sigemptyset(&blockset);
    sigaddset(&blockset, SIGUSR1);
    sigaddset(&blockset, SIGINT);
    sigprocmask(SIG_BLOCK, &blockset, NULL);
    sigaction(SIGUSR1, &signalAction, NULL);
    sigemptyset(&emptyset);

    bool stop_loop;
    client_t current_client;

    while(true){
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        max_fd = 0;

        set_server_fd(&server_fd, &max_fd, &read_fds);
        //set_admin_fd(&admin_fd, &max_fd, &read_fds);

        current_client = client_list->first;
        stop_loop = false;

        while(current_client != NULL){
            if(check_client_fds(current_client, client_list, &max_fd, &read_fds, &write_fds, settings, metrics) == -1){
                stop_loop = true;
                break;
            }
            current_client = current_client->next;
        }

        if(!stop_loop){
            res = pselect(max_fd + 1, &read_fds, &write_fds, NULL, NULL, &emptyset);
            if(res == -1 && errno != EINTR){
                perror("Error pselect");
                exit(EXIT_FAILURE);
            }
            else if (errno == EINTR){
                errno = 0;
                continue;
            }

            //resolve_admin_client(admin_fd, &read_fds, &admin_addr, admin_addr_len, settings, metrics);
            resolve_proxy_client(server_fd, &read_fds, client_list, server_addr, server_addr_len, settings, metrics);
            resolve(client_list, read_fds, write_fds, settings, metrics);
        }
    }

    return 1;
}