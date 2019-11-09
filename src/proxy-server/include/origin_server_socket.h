#ifndef ORIGIN_SERVER_SOCKET_H
#define ORIGIN_SERVER_SOCKET_H

#include <stdint.h>
#include "proxy_clients.h"
#include "settings.h"

typedef struct thread_args * thread_args_t;

struct thread_args{
    client_t client;
    pthread_t p_id;
    char *addr;
    uint16_t port;
};

int resolve_origin_server(client_t client, settings_t settings);

#endif