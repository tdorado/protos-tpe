#include "include/proxy_clients.h"
#include "include/origin_server_socket.h"
#include "include/external_transformation.h"

client_list_t init_client_list(){
    client_list_t client_list = (client_list_t)malloc(sizeof(*client_list));

    if(client_list == NULL){
        perror("Error creating client list");
        exit(EXIT_FAILURE);
    }

    client_list->first = NULL;
    client_list->last = NULL;
    client_list->qty = 0;

    return client_list;
}

client_t create_client(client_list_t client_list, const int fd){
    client_t client = (client_t)malloc(sizeof(*client));

    if(client == NULL){
        perror("Error creating client");
        return NULL;
    }

    client->client_state = NOT_LOGGED_IN;
    client->client_fd = fd;
    client->client_read_buffer = init_buffer(BUFFER_SIZE);
    client->client_write_buffer = init_buffer(BUFFER_SIZE);
    client->command_received_len = 0;
    client->logged = false;

    client->origin_server_state = NOT_RESOLVED_ORIGIN_SERVER;
    client->origin_server_fd = -1;
    client->origin_server_buffer = init_buffer(BUFFER_SIZE);
    client->received_greeting = false;

    client->external_transformation_state = PROCESS_NOT_INITIALIZED;
    client->external_transformation_read_fd = -1;
    client->external_transformation_write_fd = -1;
    client->parser_state = init_parser_state();

    client->next = NULL;

    if (client_list->qty == 0){
        client_list->first = client;
        client_list->last = client;
        client->prev = NULL;
    }
    else{
        client_list->last->next = client;
        client->prev = client_list->last;
        client_list->last = client;
    }

    client_list->qty++;

    return client;
}

void remove_client(client_list_t client_list, client_t client){
    if (client == client_list->first){
        if(client->next != NULL){
            client->next->prev = NULL;
        }
        client_list->first = client->next;
    }
    else if (client == client_list->last){
        client->prev->next = NULL;
        client_list->last = client->prev;
    }
    else{
        client->prev->next = client->next;
        client->next->prev = client->prev;
    }

    client_list->qty--;

    if (client->client_fd != -1){
        close(client->client_fd);
    }

    if (client->origin_server_fd != -1){
        close(client->origin_server_fd);
    }

    if (client->external_transformation_read_fd != -1){
        close(client->external_transformation_read_fd);
    }

    if (client->external_transformation_write_fd != -1){
        close(client->external_transformation_write_fd);
    }

    free_buffer(client->client_read_buffer);
    free_buffer(client->client_write_buffer);
    free_buffer(client->origin_server_buffer);
    free(client->parser_state);
    free(client);
}

void free_client_list(client_list_t client_list){
    client_t client = client_list->first;
    while(client != NULL){
        remove_client(client_list, client);
        client = client->next;
    }
    free(client_list);
}


void add_client(client_list_t client_list, const int proxy_fd, struct sockaddr_in6 server_addr, socklen_t * server_addr_len, settings_t settings, metrics_t metrics){
    int new_client_fd = -1;

    if ((new_client_fd = accept(proxy_fd, (struct sockaddr *)&server_addr, server_addr_len)) < 0){
        perror("Error adding client");
        return;
    }


    client_t client = create_client(client_list, new_client_fd);
    if (client == NULL){
        close(new_client_fd);
        return;
    }

    metrics->concurrent_connections++;
    metrics->total_connections++;
}

int set_client_fds(client_t client, client_list_t client_list, int *max_fd, fd_set *read_fds, fd_set *write_fds, settings_t settings, metrics_t metrics){
    if (set_external_transformation_fds(client_list, client, settings, read_fds, write_fds, metrics) == ERROR_TRANSFORMATION_PROCESS){
        return -1;
    }
    set_client_fd(client, read_fds, write_fds);

    if (set_origin_server_fd(client_list, read_fds, write_fds, client, settings, metrics) == ERROR_ORIGIN_SERVER){
        return -1;
    }

    *max_fd = max_of_five(*max_fd, client->client_fd, client->origin_server_fd, client->external_transformation_read_fd, client->external_transformation_write_fd);

    return 0;
}

void set_client_fd(client_t client, fd_set *read_fds, fd_set *write_fds){
    if (client->client_fd > 0 && client->received_greeting) {
        if (buffer_can_write(client->client_read_buffer)) {
            FD_SET(client->client_fd, read_fds);
        }

        if (buffer_can_read(client->client_write_buffer)) {
            FD_SET(client->client_fd, write_fds);
        }
    }
}

int set_origin_server_fd(client_list_t client_list, fd_set *read_fds, fd_set *write_fds, client_t client, settings_t settings, metrics_t metrics){
    if (client->origin_server_state == NOT_RESOLVED_ORIGIN_SERVER) {
        resolve_origin_server(client, settings);
    }
    else if (client->origin_server_state == ERROR_ORIGIN_SERVER) {
        send_message_to_fd(client->client_fd, ERR_ORIGIN_SERVER_CONNECTION, ERR_ORIGIN_SERVER_CONNECTION_LEN);
        remove_client(client_list, client);
        metrics->concurrent_connections--;
        return ERROR_ORIGIN_SERVER;
    }
    else if (client->origin_server_state == RESOLVED_TO_ORIGIN_SERVER && client->origin_server_fd > 0) {
        if (buffer_can_write(client->origin_server_buffer)) {
            FD_SET(client->origin_server_fd, read_fds);
        }
        
        if (buffer_can_read(client->client_read_buffer) && client->client_state != RETR_OK && client->client_state != RETR_TRANSFORMING) {
            FD_SET(client->origin_server_fd, write_fds);
        }
    }
    return 0;
}

int set_external_transformation_fds(client_list_t client_list, client_t client, settings_t settings, fd_set *read_fds, fd_set *write_fds, metrics_t metrics) {
    if (settings->transformations){
        if ( client->external_transformation_state == PROCESS_NOT_INITIALIZED ) {
            if (start_external_transformation_process(settings, client) == ERROR_TRANSFORMATION_PROCESS) {
                send_message_to_fd(client->client_fd, ERR_TRANSFORMATION, ERR_TRANSFORMATION_LEN);
                remove_client(client_list, client);
                metrics->concurrent_connections--;;
                return ERROR_TRANSFORMATION_PROCESS;
            }
            client->external_transformation_state = PROCESS_INITIALIZED;
        }
    }
    if((settings->transformations && client->client_state == RETR_OK) || client->client_state == RETR_TRANSFORMING) {
        if (client->external_transformation_read_fd != -1 && buffer_can_write(client->client_write_buffer)) {
            FD_SET(client->external_transformation_read_fd, read_fds);
        }

        if (client->external_transformation_write_fd != -1 && buffer_can_read(client->origin_server_buffer)) {
            FD_SET(client->external_transformation_write_fd, write_fds);
        }
    }

    return 0;
}

pop_response_t get_response(buffer_t buffer) {
    if (*(buffer->read) == '+') {
        return OK_RESPONSE;
    }
    return ERR_RESPONSE;
}

void resolve_client(client_t client, client_list_t client_list, fd_set *read_fds, fd_set *write_fds, settings_t settings, metrics_t metrics) {
    int bytes_read;

    if (FD_ISSET(client->client_fd, read_fds)) {
        bytes_read = read_from_fd(client->client_fd, client->client_read_buffer);

        if (bytes_read == 0) {
            remove_client(client_list, client);
            metrics->concurrent_connections--;
            return;
        }

        if(bytes_read >= 4){
            if (strncasecmp((char *)client->client_read_buffer->read, "retr", 4) == 0 && client->client_state == LOGGED_IN) {
                client->client_state = RETR_REQUEST;
            }
            else if (strncasecmp((char *)client->client_read_buffer->read, "pass", 4) == 0) {
                client->client_state = PASS_REQUEST;
            }
            else if (strncasecmp((char *)client->client_read_buffer->read, "capa", 4) == 0) {
                client->client_state = CAPA_REQUEST;
            }
        }
        else{
            int aux = 0;
            while(client->command_received_len < 4 && aux < bytes_read ){
                client->command_received[client->command_received_len] = *((char*)client->client_read_buffer->read + aux);
                aux++;
                client->command_received_len++;
            }
            if(client->command_received_len == 4){
                if (strncasecmp(client->command_received, "retr", 4) == 0 && client->client_state == LOGGED_IN) {
                    client->client_state = RETR_REQUEST;
                }
                else if (strncasecmp(client->command_received, "pass", 4) == 0) {
                    client->client_state = PASS_REQUEST;
                }
                else if (strncasecmp(client->command_received, "capa", 4) == 0) {
                    client->client_state = CAPA_REQUEST;
                }
                client->command_received_len = 0;
            }
        }

        metrics->bytes_transfered += bytes_read;
    }

    if (FD_ISSET(client->client_fd, write_fds)) {
        write_to_fd(client->client_fd, client->client_write_buffer);
    }

    if (client->origin_server_state == RESOLVED_TO_ORIGIN_SERVER) {
        if (FD_ISSET(client->origin_server_fd, write_fds)) {
            write_until_enter_to_fd(client->origin_server_fd, client->client_read_buffer);
        }

        if (FD_ISSET(client->origin_server_fd, read_fds)) {
            bytes_read = read_from_fd(client->origin_server_fd, client->origin_server_buffer);

            if (bytes_read == 0) {
                remove_client(client_list, client);
                metrics->concurrent_connections--;
            }

            if(client->received_greeting){
                if (get_response(client->origin_server_buffer) == OK_RESPONSE) {
                    if (client->client_state == PASS_REQUEST) {
                        client->client_state = LOGGED_IN;
                        client->logged = true;
                    }
                    else if(client->client_state == RETR_REQUEST){
                        client->client_state = RETR_OK;
                    }
                    else if(client->client_state == CAPA_REQUEST){
                        if(client->logged){
                            client->client_state = LOGGED_IN;
                        }
                        else{
                            client->client_state = NOT_LOGGED_IN;
                        }
                        char aux[bytes_read + 13];
                        for(int i = 0; i < bytes_read; i++){
                            aux[i] = buffer_read(client->origin_server_buffer);
                        }
                        buffer_reset(client->origin_server_buffer);
                        aux[bytes_read] = '\0';
                        if(strstr(aux, "PIPELINING") == NULL){
                            strcpy(aux + bytes_read - 3, "PIPELINING\r\n.\r\n");
                            bytes_read += 12;
                        }
                        for(int i = 0; i < bytes_read; i++){
                            buffer_write(client->origin_server_buffer, aux[i]);
                        }
                    }
                }
            }
            else{
                if (get_response(client->origin_server_buffer) == OK_RESPONSE) {
                    send_message_to_fd(client->client_fd, OK_WELCOME, OK_WELCOME_LEN);
                    buffer_reset(client->origin_server_buffer);
                    client->received_greeting = true;
                }
                else{
                    send_message_to_fd(client->client_fd, ERR_ORIGIN_SERVER_CONNECTION, ERR_ORIGIN_SERVER_CONNECTION_LEN);
                    remove_client(client_list, client);
                    metrics->concurrent_connections--;
                }
            }

            metrics->bytes_transfered += bytes_read;
        }

        if ((settings->transformations && client->client_state == RETR_OK) || client->client_state == RETR_TRANSFORMING) {
            if (client->external_transformation_state == PROCESS_INITIALIZED) {
                if (client->client_state == RETR_OK) {
                    // Send response line to client write buffer
                    uint8_t c;
                    while (buffer_can_read(client->origin_server_buffer) && (c = buffer_read(client->origin_server_buffer)) != '\r') {
                        buffer_write(client->client_write_buffer, c);
                    }
                    buffer_write(client->client_write_buffer, buffer_read(client->origin_server_buffer));                    
                    client->client_state = RETR_TRANSFORMING;
                }
                if (!buffer_can_read(client->origin_server_buffer)) {
                    close(client->external_transformation_write_fd);
                }
                if (FD_ISSET(client->external_transformation_write_fd, write_fds)) {
                    write_and_parse_to_fd(client->external_transformation_write_fd, client->origin_server_buffer, client->parser_state);
                }
                if (FD_ISSET(client->external_transformation_read_fd, read_fds)) {

                    bytes_read = read_and_parse_from_fd(client->external_transformation_read_fd, client->client_write_buffer, client->parser_state);
                    
                    if (bytes_read == 0) {
                        close(client->external_transformation_read_fd);
                        close(client->external_transformation_write_fd);
                        client->external_transformation_read_fd = -1;
                        client->external_transformation_write_fd = -1;
                        client->external_transformation_state = PROCESS_NOT_INITIALIZED;
                        reset_parser_state(client->parser_state);

                        write_to_fd(client->client_fd, client->client_write_buffer);
                        send_message_to_fd(client->client_fd, CRLF_DOT_CRLF, CRLF_DOT_CRLF_LEN);
                        client->client_state = LOGGED_IN;
                    }
                }
            }
        }
        else{
            buffer_copy(client->origin_server_buffer, client->client_write_buffer);
        }
    }
}