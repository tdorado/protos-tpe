#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "include/origin_server_socket.h"
#include "include/logs.h"

typedef struct thread_args * thread_args_t;

struct thread_args {
    client_t client;
    pthread_t p_id;
    char *origin_server_addr;
    uint16_t origin_server_port;
};

static void * resolve_origin_server_thread(void * args);

int resolve_origin_server(client_t client, settings_t settings) {
    client->origin_server_state = RESOLVING_ORIGIN_SERVER;
    thread_args_t thread_args = (thread_args_t)malloc(sizeof(*thread_args));
    if (thread_args == NULL){
        perror("Error creating threadArgs");
        return -1;
    }

    thread_args->client = client;
    thread_args->p_id = pthread_self();
    thread_args->origin_server_addr = settings->origin_server_addr;
    thread_args->origin_server_port = settings->origin_server_port;

    pthread_t thread;

    if (pthread_create(&thread, NULL, resolve_origin_server_thread, (void *)thread_args) == -1) {
        perror("Error creating new thread");
        return -1;
    }

    return 1;
}

static void * resolve_origin_server_thread(void *args) {
    thread_args_t thread_args = (thread_args_t)args;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res;

    if (getaddrinfo(thread_args->origin_server_addr, NULL, &hints, &res) != 0) {
        thread_args->client->origin_server_state = ERROR_ORIGIN_SERVER;
        pthread_kill(thread_args->p_id, SIGUSR1);
        pthread_exit(NULL);
    }

    struct addrinfo * rp;
    int origin_server_fd;
    socklen_t origin_server_addr_len;
    struct sockaddr_in * origin_server_addr;

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        origin_server_addr = (struct sockaddr_in *)rp->ai_addr;
        origin_server_addr->sin_port = htons(thread_args->origin_server_port);

        if (origin_server_addr->sin_family == AF_INET6) {
            origin_server_addr_len = sizeof(struct sockaddr_in6);
        } else {
            origin_server_addr_len = sizeof(struct sockaddr_in);
        }

        origin_server_fd = socket(origin_server_addr->sin_family, SOCK_STREAM, 0);

        if (connect(origin_server_fd, (struct sockaddr *)origin_server_addr, origin_server_addr_len) == -1) {
            perror("Error connecting to Origin Server");
        } else {
            thread_args->client->origin_server_fd = origin_server_fd;
            thread_args->client->origin_server_state = RESOLVED_TO_ORIGIN_SERVER;
            break;
        }

        close(origin_server_fd);
    }

    if (rp == NULL) {
        perror("Error connecting to Origin Server\n");
        thread_args->client->origin_server_state = ERROR_ORIGIN_SERVER;
        pthread_kill(thread_args->p_id, SIGUSR1);
        pthread_exit(NULL);
    }

    freeaddrinfo(res);
    free(thread_args);
    pthread_kill(thread_args->p_id, SIGUSR1);
    pthread_detach(pthread_self());
    pthread_exit(NULL);
    return NULL;
}

static void * get_pipelining_origin_server_thread(void *args) {
    settings_t settings = (settings_t)args;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res;

    if (getaddrinfo(settings->origin_server_addr, NULL, &hints, &res) != 0) {
        perror("Error connecting to Origin Server\n");
        settings->pipelining = false;
        log_message(false, "Could not get origin Server pipelining, assume it doesn't");
        pthread_exit(NULL);
    }

    struct addrinfo *rp;
    int origin_server_fd;
    socklen_t origin_server_addr_len;
    struct sockaddr_in * origin_server_addr;

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        origin_server_addr = (struct sockaddr_in *)rp->ai_addr;
        origin_server_addr->sin_port = htons(settings->origin_server_port);

        if (origin_server_addr->sin_family == AF_INET6) {
            origin_server_addr_len = sizeof(struct sockaddr_in6);
        } else {
            origin_server_addr_len = sizeof(struct sockaddr_in);
        }

        origin_server_fd = socket(origin_server_addr->sin_family, SOCK_STREAM, 0);

        if (connect(origin_server_fd, (struct sockaddr *)origin_server_addr, origin_server_addr_len) != -1) {
            break;
        }

        close(origin_server_fd);
    }

    if (rp == NULL) {
        perror("Error connecting to Origin Server\n");
        settings->pipelining = false;
        log_message(false, "Could not get origin Server pipelining, assume it doesn't");
        pthread_exit(NULL);
    }

    char buffer[500];
    int len;

    len = read(origin_server_fd, buffer, 500);
    if(len > 0 && buffer[0] == '+'){
        write(origin_server_fd, "CAPA\n", 5);
        len = read(origin_server_fd, buffer, 500);
        buffer[len] = '\0';
        if(strstr(buffer, "\nPIPELINING") != NULL){
            settings->pipelining = true;
            log_message(false, "Origin Server has pipelining");
        }
        else{
            settings->pipelining = false;
            log_message(false, "Origin Server does not have pipelining");
        }
    }

    close(origin_server_fd);
    freeaddrinfo(res);
    pthread_detach(pthread_self());
    pthread_exit(NULL);
    return NULL;
}

void get_pipelining_origin_server(settings_t settings) {
    pthread_t thread;

    if (pthread_create(&thread, NULL, get_pipelining_origin_server_thread, (void *)settings) == -1) {
        perror("Error creating pipelining thread");
        exit(EXIT_FAILURE);
    }
}
