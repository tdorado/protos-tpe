#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "include/input_parser.h"

#define BUFFER_SIZE 2048

// Variable con toda la informacion del proxy, esto despues modificaria el administrator
static input_t proxy_params;

void pop3_handle_connection(const int client, struct sockaddr* client_addr) {
    bool exit = false;
    char buffer[BUFFER_SIZE];
    //HABRIA QUE VER COMO HACER BIEN LA BUFFER. UNA COLA DE BUFFERS QUIZAS?

    struct sockaddr_in origin_pop_addr;

    memset(&origin_pop_addr, 0, sizeof(origin_pop_addr));

    origin_pop_addr.sin_family      = AF_INET;
    origin_pop_addr.sin_addr.s_addr = inet_addr(proxy_params->origin_server_addr);
    origin_pop_addr.sin_port        = htons(proxy_params->origin_server_port);
    int origin_server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (connect(origin_server_fd, (struct sockaddr *)&origin_pop_addr, sizeof(origin_pop_addr)) < 0) 
    { 
        perror("Connection to origin POP3 Server Failed. \n");
        close(client);
        return; 
    }

    int n = recv(origin_server_fd, buffer, BUFFER_SIZE, 0);
    while (!exit) {
        send(client, buffer, n, 0);
        n = recv(client, buffer, BUFFER_SIZE, 0);

        //FALTA CONECTAR CON EL PARSER DE ENTRADA DE LA OTRA BRANCH

        send(origin_server_fd, buffer, n, 0);
        n = recv(origin_server_fd, buffer, BUFFER_SIZE, 0);
        
        //HAY QUE VER LO DE LA BUFFER, COLA DE BUFFERS QUIZAS?
        //ACA SE HARIA LA TRANSFORMACION EXTERNA

        //SOLUCION TEMPORAL PARA EL QUIT O CUANDO SE PIERDE CONEXION, ETC
        //ESTO VA A SER DIFERENTE CON EL PARSER EL INPUT DE POP3
        if(n == 0 || (strncmp(buffer, "+OK Logging out", 15) == 0)){
            exit = true;
            send(client, buffer, n, 0);
        }
    }
    close(origin_server_fd);
    close(client);
}

/**
 * estructura utilizada para transportar datos entre el hilo
 * que acepta sockets y los hilos que procesa cada conexión
 */
struct connection {
    int fd;
    socklen_t addrlen;
    struct sockaddr_in6 addr;
};

/** rutina de cada hilo worker */
static void * handle_connection_pthread(void *args) {
    const struct connection *c = args;
    pthread_detach(pthread_self());

    pop3_handle_connection(c->fd, (struct sockaddr *)&c->addr);
    free(args);

    return 0;
}

int serve_pop3_connections(const int server) {
    while (1) {
        struct sockaddr_in6 caddr;
        socklen_t caddrlen = sizeof (caddr);
        // Wait for a client to connect
        const int client = accept(server, (struct sockaddr*)&caddr, &caddrlen);
        if (client < 0) {
            perror("Unable to accept incoming socket. \n");
        }
        else {
            // TODO: limitar la cantidad de hilos concurrentes
            struct connection* c = malloc(sizeof (struct connection));
            if (c == NULL) {
                // lo trabajamos iterativamente
                pop3_handle_connection(client, (struct sockaddr*)&caddr);
            }
            else {
                pthread_t tid;
                c->fd = client;
                c->addrlen = caddrlen;
                memcpy(&(c->addr), &caddr, caddrlen);
                if (pthread_create(&tid, 0, handle_connection_pthread, c)) {
                    free(c);
                    // lo trabajamos iterativamente
                    pop3_handle_connection(client, (struct sockaddr*)&caddr);
                }
            }
        }
    }
    return 0;
}

int main(int argc, char ** argv) {

    struct input_params params;
    proxy_params = &params;
    
    if (input_parser(argc, argv, proxy_params) < 0) {
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(proxy_params->local_addr);
    addr.sin_port        = htons(proxy_params->local_port);

    const int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server < 0) {
        perror("Unable to create socket. \n");
        return 1;
    }

    fprintf(stdout, "Listening on TCP port %d\n", proxy_params->local_port);

    // man 7 ip. no importa reportar nada si falla.
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    if(bind(server, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("Unable to bind socket. \n");
        return 1;
    }

    if (listen(server, 20) < 0) {
        perror("Unable to listen. \n");
        return 1;
    }

    int ret = serve_pop3_connections(server);

    if(server >= 0) {
        close(server);
    }

    return ret;
}