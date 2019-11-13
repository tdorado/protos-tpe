#ifndef ORIGIN_SERVER_SOCKET_H
#define ORIGIN_SERVER_SOCKET_H

#include <stdint.h>
#include "proxy_clients.h"
#include "settings.h"

int resolve_origin_server(client_t client, settings_t settings);
void get_pipelining_origin_server(settings_t settings);

#endif