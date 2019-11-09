#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include "include/origin_server_socket.h"

static void *resolve_origin_server_thread(void *args);

int resolve_origin_server(client_t client, settings_t settings) {
    client->origin_server_state = RESOLVING_ORIGIN_SERVER;
    thread_args_t thread_args = (thread_args_t)malloc(sizeof(*thread_args));
    if (thread_args == NULL){
        perror("Error creating threadArgs");
        exit(EXIT_FAILURE);
    }

    thread_args->client = client;
    thread_args->p_id = pthread_self();
    thread_args->addr = settings->origin_server_addr;
    thread_args->port = settings->origin_server_port;

    pthread_t thread;

    if (pthread_create(&thread, NULL, resolve_origin_server_thread, (void *)thread_args) == -1){
        perror("Error creating new thread");
        exit(EXIT_FAILURE);
    }

    return 1;
}

static void *resolve_origin_server_thread(void *args) {
    thread_args_t thread_args = (thread_args_t)args;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; // We want IPv4 and IPv6 addresses
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res;

    int result = getaddrinfo(thread_args->addr, NULL, &hints, &res);

    if (result != 0) {
        thread_args->client->origin_server_state = ERROR_ORIGIN_SERVER;
        pthread_kill(thread_args->p_id, SIGUSR1);
        pthread_exit(NULL);
    }

    /* 
    * getaddrinfo() returns a list of address structures.
    * Try each address until we successfully connect(2).
    * If socket(2) (or connect(2)) fails, we (close the socket
    * and) try the next address.
    */
    struct addrinfo *rp;
    int socketFD;
    /* Setting defaults for IPv4 */
    int originServerDomain = AF_INET;
    unsigned int addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in *saddr;

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        saddr = (struct sockaddr_in *)rp->ai_addr;
        saddr->sin_port = htons(thread_args->port);

        if (saddr->sin_family == AF_INET6) {
            originServerDomain = AF_INET6;
            addrlen = sizeof(struct sockaddr_in6);
        }

        if ((socketFD = socket(originServerDomain, SOCK_STREAM, 0)) == -1) {
            continue;
        }

        if (connect(socketFD, (struct sockaddr *)saddr, addrlen) == -1) {
            perror("Error connecting to Origin Server");
        }
        else {
            thread_args->client->origin_server_fd = socketFD;
            thread_args->client->origin_server_state = CONNECTED_TO_ORIGIN_SERVER;
            break;
        }

        /* Solamente llega acá si falló con el actual rp. Vuelve a intentar con el próximo */
        close(socketFD);
    }

    if (rp == NULL) {
        /* No address succeeded */
        perror("Error connecting to Origin Server\n");
        thread_args->client->origin_server_state = ERROR_ORIGIN_SERVER;
        pthread_kill(thread_args->p_id, SIGUSR1);
        pthread_exit(NULL);
    }

    /* No longer needed */
    freeaddrinfo(res);
    
    pthread_kill(thread_args->p_id, SIGUSR1);
    pthread_exit(EXIT_SUCCESS);
    printf("prueba\n");
    return NULL;
}