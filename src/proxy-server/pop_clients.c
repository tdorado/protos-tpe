#include "include/pop_clients.h"
#include "include/origin_server.h"
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

    client->client_state = CONNECTED;
    client->client_fd = fd;
    client->client_read_buffer = initialize_buffer(BUFFER_SIZE);
    client->client_write_buffer = initialize_buffer(BUFFER_SIZE);
    client->logged = false;

    client->origin_server_state = NOT_RESOLVED;
    client->origin_server_fd = -1;
    client->origin_server_read_buffer = initialize_buffer(BUFFER_SIZE);
    client->origin_server_write_buffer = initialize_buffer(BUFFER_SIZE);
    client->received_greeting = false;

    client->external_transformation_state = PROCESS_NOT_INITIALIZED;
    client->external_transformation_read_fd = -1;
    client->external_transformation_write_fd = -1;
    client->transformation_buffer = initialize_buffer(BUFFER_SIZE);

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
    free_buffer(client->origin_server_read_buffer);
    free_buffer(client->origin_server_write_buffer);
    free_buffer(client->transformation_buffer);

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


void accept_new_client(client_list_t client_list, const int server_fd, struct sockaddr_in6 server_addr, socklen_t server_addr_len, settings_t settings, metrics_t metrics){
    int new_client_fd = -1;

    if ((new_client_fd = accept(server_fd, (struct sockaddr *)&server_addr, (socklen_t *)&server_addr_len)) < 0){
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

int check_client_fds(client_t client, client_list_t client_list, int *max_fd, fd_set *read_fds, fd_set *write_fds, settings_t settings, metrics_t metrics){
    
    if (settings->transformations){
        if (check_external_transformation_fds(client_list, client, settings, read_fds, write_fds, metrics) == ERROR_TRANSFORMATION){
            return ERROR_TRANSFORMATION;
        }
    }

    *max_fd = max(*max_fd, client->client_fd, client->origin_server_fd, client->external_transformation_read_fd, client->external_transformation_write_fd);

    if (client->client_fd > 0 && client->origin_server_state == CONNECTED_TO_ORIGIN_SERVER) {
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

    if (client->origin_server_state == NOT_RESOLVED) {
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
        if (buffer_can_write(client->origin_server_read_buffer)) {
            FD_SET(client->origin_server_fd, read_fds);
        }
        
        if (buffer_can_read(client->origin_server_write_buffer)) {
            FD_SET(client->origin_server_fd, write_fds);
        }
    }

    return 0;
}

void resolve_client(client_t client, client_list_t client_list, fd_set *read_fds, fd_set *write_fds, settings_t settings, metrics_t metrics) {
    long client_read_buffer_bytes;
    long origin_server_read_bytes;

    /* Client wrote to the pop3filter. */
    if (FD_ISSET(client->client_fd, read_fds)) {
        client_read_buffer_bytes = read_from_fd(&client->client_fd, client->client_read_buffer);

        /* User wants to disconnet. */
        if (client_read_buffer_bytes == 0) {
            remove_client(client_list, client);
            metrics->concurrent_connections--;
            return;
        }

        interpret_request(client);

        metrics->bytes_transfered += client_read_buffer_bytes;
    }

    /* pop3filter can write to the client. */
    if (FD_ISSET(client->client_fd, write_fds)) {
        write_to_fd(&client->client_fd, client->client_write_buffer);
    }

    buffer_move(client->client_read_buffer, client->origin_server_write_buffer);

    if (client->origin_server_state == CONNECTED_TO_ORIGIN_SERVER) {
        /* pop3filter can write to the origin server. */
        if (FD_ISSET(client->origin_server_fd, write_fds)) {
            write_to_fd(&client->origin_server_fd, client->origin_server_write_buffer);
        }
        /* Origin server wrote to the pop3filter. */
        if (FD_ISSET(client->origin_server_fd, read_fds)) {
            origin_server_read_bytes = read_from_fd(&client->origin_server_fd, client->origin_server_read_buffer);

            if (origin_server_read_bytes == 0) {
                /* Origin server wants to disconnect. */
                remove_client(client_list, client);
                metrics->concurrent_connections--;
            }

            if(client->received_greeting){
                if (interpret_response(client->origin_server_read_buffer) == OK_RESPONSE) {
                    switch (client->client_state) {
                        case USER_REQUEST:
                            client->client_state = USER_OK;
                            break;

                        case PASS_REQUEST:
                            client->client_state = PASS_OK;
                            client->logged = true;
                            break;

                        case RETR_REQUEST:
                            client->client_state = RETR_OK;
                            break;

                        default:
                            client->client_state = OTHER_REQUEST_OK;
                            break;
                    }
                }
                else{
                    client->client_state = CONNECTED;
                }
            }
            else{
                if (interpret_response(client->origin_server_read_buffer) == OK_RESPONSE) {
                    send_message_to_fd(&client->client_fd, "+OK Welcome\r\n", 13);
                    buffer_reset(client->origin_server_read_buffer);
                    client->received_greeting = true;
                }
                else{
                    send_message_to_fd(&client->client_fd, "-ERR Connection refused\r\n", 25);
                    remove_client(client_list, client);
                    metrics->concurrent_connections--;
                }
            }

            metrics->bytes_transfered += origin_server_read_bytes;
        }

        if (settings->transformations && (client->client_state == RETR_OK || client->client_state == REMOVING_LAST_LINE || client->client_state == RETR_TRANSFORMING)) {
            if (!buffer_can_read(client->origin_server_read_buffer)) {
                close(client->external_transformation_write_fd);
            }

            if (client->external_transformation_state == PROCESS_INITIALIZED) {
                if (FD_ISSET(client->external_transformation_read_fd, read_fds)) {

                    origin_server_read_bytes = read_from_fd(&client->external_transformation_read_fd, client->transformation_buffer);

                    if (origin_server_read_bytes == -1) {
                        perror("Error reading from transformation process");
                        return;
                    }
                    else if (origin_server_read_bytes == 0) {
                        /* Transformation wants to disconnect. */
                        close(client->external_transformation_read_fd);
                        close(client->external_transformation_write_fd);
                        client->external_transformation_read_fd = -1;
                        client->external_transformation_write_fd = -1;
                        client->external_transformation_state = PROCESS_NOT_INITIALIZED;

                        client->client_state = RETR_FINISHED_TRANSFORMING;
                    }

                    buffer_move(client->transformation_buffer, client->client_write_buffer);
                    if(client->client_state == RETR_FINISHED_TRANSFORMING){
                        write_to_fd(&client->client_fd, client->client_write_buffer);
                        send_message_to_fd(&client->client_fd, "\r\n.\r\n", 5);
                        client->client_state = CONNECTED;
                    }
                }

                if (client->client_state == RETR_OK) {
                    move_response_line(client->origin_server_read_buffer, client->client_write_buffer);
                    client->client_state = REMOVING_LAST_LINE;
                    remove_crlf_dot_crlf(client->origin_server_read_buffer);
                }

                if (client->client_state == REMOVING_LAST_LINE) {
                    if (remove_crlf_dot_crlf(client->origin_server_read_buffer)) {
                        client->client_state = RETR_TRANSFORMING;
                    }
                }

                if (FD_ISSET(client->external_transformation_write_fd, write_fds)) {
                    write_to_fd(&client->external_transformation_write_fd, client->origin_server_read_buffer);
                }
            }
        }
        else{
            buffer_move(client->origin_server_read_buffer, client->client_write_buffer);
        }
    }
}

int check_external_transformation_fds(client_list_t client_list, client_t client, settings_t settings, fd_set *read_fds, fd_set *write_fds, metrics_t metrics) {
    if ( client->external_transformation_state == PROCESS_NOT_INITIALIZED ) {
        if (start_external_transformation_process(settings, client) == ERROR_TRANSFORMATION) {
            send_message_to_fd(&client->client_fd, "-ERR Transformation refused\r\n", 29);
            remove_client(client_list, client);
            metrics->concurrent_connections--;;
            return ERROR_TRANSFORMATION;
        }
        client->external_transformation_state = PROCESS_INITIALIZED;
    }
    else {
        if (client->external_transformation_read_fd != -1 && buffer_can_write(client->transformation_buffer)) {
            FD_SET(client->external_transformation_read_fd, read_fds);
        }

        if (client->external_transformation_write_fd != -1 && buffer_can_read(client->origin_server_read_buffer)) {
            FD_SET(client->external_transformation_write_fd, write_fds);
        }
    }

    return 0;
}


void interpret_request(client_t client) {
    if (client->client_read_buffer->write - client->client_read_buffer->read >= 4) {
        char * command = (char *)client->client_read_buffer->read;

        if (strncasecmp(command, "retr", 4) == 0 && client->logged) {
            client->client_state = RETR_REQUEST;
        }
        else if (strncasecmp(command, "user", 4) == 0) {
            client->client_state = USER_REQUEST;
        }
        else if (strncasecmp(command, "pass", 4) == 0) {
            client->client_state = PASS_REQUEST;
        }
        else{
            client->client_state = OTHER_REQUEST;
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