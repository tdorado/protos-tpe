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
    client->logged = false;

    client->origin_server_state = NOT_RESOLVED_ORIGIN_SERVER;
    client->origin_server_fd = -1;
    client->origin_server_buffer = init_buffer(BUFFER_SIZE);
    client->received_greeting = false;

    client->external_transformation_state = PROCESS_NOT_INITIALIZED;
    client->external_transformation_read_fd = -1;
    client->external_transformation_write_fd = -1;

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
        perror("Error accepting new client");
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
    if (set_external_transformation_fds(client_list, client, settings, read_fds, write_fds, metrics) == ERROR_TRANSFORMATION){
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
        // Did the client write to us?
        // This writes the client request to the clientRead buffer
        // clientRead it's because we are READING from the client
        if (buffer_can_write(client->client_read_buffer)) {
            FD_SET(client->client_fd, read_fds);
        }

        /* Can we write to the client? */
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
        send_message_to_fd(&client->client_fd, "-ERR Connection refused\r\n", 25);
        remove_client(client_list, client);
        metrics->concurrent_connections--;
        return ERROR_ORIGIN_SERVER;
    }
    else if (client->origin_server_state == CONNECTED_TO_ORIGIN_SERVER && client->origin_server_fd > 0) {
        /* Did the server write to us? */
        if (buffer_can_write(client->origin_server_buffer)) {
            FD_SET(client->origin_server_fd, read_fds);
        }
        
        if (buffer_can_read(client->client_read_buffer)) {
            FD_SET(client->origin_server_fd, write_fds);
        }
    }
    return 0;
}

int set_external_transformation_fds(client_list_t client_list, client_t client, settings_t settings, fd_set *read_fds, fd_set *write_fds, metrics_t metrics) {
    if (settings->transformations){
        if ( client->external_transformation_state == PROCESS_NOT_INITIALIZED ) {
            if (start_external_transformation_process(settings, client) == ERROR_TRANSFORMATION) {
                send_message_to_fd(&client->client_fd, "-ERR Transformation refused\r\n", 29);
                remove_client(client_list, client);
                metrics->concurrent_connections--;;
                return ERROR_TRANSFORMATION;
            }
            client->external_transformation_state = PROCESS_INITIALIZED;
        }
    }
    if((settings->transformations && client->client_state == RETR_OK) || client->client_state == RETR_TRANSFORMING || client->client_state == REMOVING_LAST_LINE) {
        if (client->external_transformation_read_fd != -1 && buffer_can_write(client->client_write_buffer)) {
            FD_SET(client->external_transformation_read_fd, read_fds);
        }

        if (client->external_transformation_write_fd != -1 && buffer_can_read(client->origin_server_buffer)) {
            FD_SET(client->external_transformation_write_fd, write_fds);
        }
    }

    return 0;
}

void resolve_client(client_t client, client_list_t client_list, fd_set *read_fds, fd_set *write_fds, settings_t settings, metrics_t metrics) {
    int bytes_read;

    /* Client wrote to the pop3filter. */
    if (FD_ISSET(client->client_fd, read_fds)) {
        bytes_read = read_from_fd(&client->client_fd, client->client_read_buffer);

        /* User wants to disconnet. */
        if (bytes_read == 0) {
            remove_client(client_list, client);
            metrics->concurrent_connections--;
            return;
        }

        interpret_request(client);

        metrics->bytes_transfered += bytes_read;
    }

    /* pop3filter can write to the client. */
    if (FD_ISSET(client->client_fd, write_fds)) {
        write_to_fd(&client->client_fd, client->client_write_buffer);
    }

    if (client->origin_server_state == CONNECTED_TO_ORIGIN_SERVER) {
        /* pop3filter can write to the origin server. */
        if (FD_ISSET(client->origin_server_fd, write_fds)) {
            write_to_fd(&client->origin_server_fd, client->client_read_buffer);
        }
        /* Origin server wrote to the pop3filter. */
        if (FD_ISSET(client->origin_server_fd, read_fds)) {
            bytes_read = read_from_fd(&client->origin_server_fd, client->origin_server_buffer);

            if (bytes_read == 0) {
                /* Origin server wants to disconnect. */
                remove_client(client_list, client);
                metrics->concurrent_connections--;
            }

            if(client->received_greeting){
                if (interpret_response(client->origin_server_buffer) == OK_RESPONSE) {
                    if (client->client_state == PASS_REQUEST) {
                        client->client_state = LOGGED_IN;
                        client->logged = true;
                    }
                    else if(client->client_state == RETR_REQUEST){
                        client->client_state = RETR_OK;
                    }
                }
            }
            else{
                if (interpret_response(client->origin_server_buffer) == OK_RESPONSE) {
                    send_message_to_fd(&client->client_fd, "+OK Welcome\r\n", 13);
                    buffer_reset(client->origin_server_buffer);
                    client->received_greeting = true;
                }
                else{
                    send_message_to_fd(&client->client_fd, "-ERR Connection refused\r\n", 25);
                    remove_client(client_list, client);
                    metrics->concurrent_connections--;
                }
            }

            metrics->bytes_transfered += bytes_read;
        }

        if ((settings->transformations && client->client_state == RETR_OK) || client->client_state == REMOVING_LAST_LINE || client->client_state == RETR_TRANSFORMING) {
            if (client->external_transformation_state == PROCESS_INITIALIZED) {
                if (FD_ISSET(client->external_transformation_read_fd, read_fds)) {

                    bytes_read = read_from_fd(&client->external_transformation_read_fd, client->client_write_buffer);

                    if (bytes_read == -1) {
                        perror("Error reading from transformation process");
                        return;
                    }
                    else if (bytes_read == 0) {
                        /* Transformation wants to disconnect. */
                        close(client->external_transformation_read_fd);
                        close(client->external_transformation_write_fd);
                        client->external_transformation_read_fd = -1;
                        client->external_transformation_write_fd = -1;
                        client->external_transformation_state = PROCESS_NOT_INITIALIZED;

                        write_to_fd(&client->client_fd, client->client_write_buffer);
                        send_message_to_fd(&client->client_fd, "\r\n.\r\n", 5);
                        client->client_state = LOGGED_IN;
                    }
                }

                if (client->client_state == RETR_OK) {
                    move_response_line(client->origin_server_buffer, client->client_write_buffer);
                    client->client_state = REMOVING_LAST_LINE;
                    remove_crlf_dot_crlf(client->origin_server_buffer);
                }

                if (client->client_state == REMOVING_LAST_LINE) {
                    if (remove_crlf_dot_crlf(client->origin_server_buffer)) {
                        client->client_state = RETR_TRANSFORMING;
                    }
                }
                if (!buffer_can_read(client->origin_server_buffer)) {
                    close(client->external_transformation_write_fd);
                }
                if (FD_ISSET(client->external_transformation_write_fd, write_fds)) {
                    write_to_fd(&client->external_transformation_write_fd, client->origin_server_buffer);
                }
            }
        }
        else{
            buffer_move(client->origin_server_buffer, client->client_write_buffer);
        }
    }
}

void interpret_request(client_t client) {
    if (client->client_read_buffer->write - client->client_read_buffer->read >= 4) {
        char * command = (char *)client->client_read_buffer->read;

        if (strncasecmp(command, "retr", 4) == 0 && client->logged) {
            client->client_state = RETR_REQUEST;
        }
        else if (strncasecmp(command, "pass", 4) == 0) {
            client->client_state = PASS_REQUEST;
        }
    }
}

int interpret_response(buffer_t buff) {
    if (*(buff->read) == '+') {
        return OK_RESPONSE;
    }
    return ERR_RESPONSE;
}

void move_response_line(buffer_t from, buffer_t to) {
    uint8_t character;
    /* Saco la primer linea del pop3 */
    while (buffer_can_read(from) && (character = buffer_read(from)) != '\r') {
        buffer_write(to, character);
    }
    buffer_write(to, buffer_read(from));
}

int remove_crlf_dot_crlf(buffer_t buffer) {
    /* Saco \r\n.\r\n */
    char *ptr;
    ptr = strstr((char *)buffer->read, "\r\n.\r\n");

    if (ptr == NULL){
        return 0;
    }

    buffer->write = (uint8_t *)ptr;
    return 1;
}