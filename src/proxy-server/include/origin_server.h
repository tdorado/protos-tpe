#ifndef ORIGIN_SERVER
#define ORIGIN_SERVER

#include "pop_clients.h"
#include "settings.h"

struct thread_args{
    client_t client;
    pthread_t p_id;
    char *addr;
    uint16_t port;
};

typedef struct thread_args * thread_args_t;

int resolve_origin_server(client_t client, settings_t settings);

#endif