#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "include/pop_parser.h"
#include "include/proxy_clients.h"

transformation_parser_state_t init_transformation_parser_state() {
    transformation_parser_state_t ret = (transformation_parser_state_t)malloc(sizeof(*ret));
    if(ret == NULL) {
        perror("Error creating parser state for client");
        return NULL;
    }

    ret->in_ps.buffer = init_buffer(BUFFER_SIZE);
    if(ret->in_ps.buffer == NULL){
        free(ret);
        perror("Error creating parser state for client");
        return NULL;
    }

    reset_transformation_parser_state(ret);

    return ret;
}

void reset_transformation_parser_state(transformation_parser_state_t transformation_parser_state) {
    transformation_parser_state->out_ps.first_r_found = false;
    transformation_parser_state->out_ps.first_n_found = false;
    transformation_parser_state->out_ps.dot_found = false;
    transformation_parser_state->out_ps.second_r_found = false;

    buffer_reset(transformation_parser_state->in_ps.buffer);
    transformation_parser_state->in_ps.r_found = false;
    transformation_parser_state->in_ps.n_found = false;
    transformation_parser_state->in_ps.last_char = 0;
}

void free_transformation_parser_state (transformation_parser_state_t transformation_parser_state) {
    free_buffer(transformation_parser_state->in_ps.buffer);
    free(transformation_parser_state);
}

ssize_t write_and_parse_transformation_to_fd(int fd, buffer_t buffer, transformation_parser_state_t transformation_parser_state) {
    char c;
    ssize_t ret = 0;
    bool puts_c = true;
    bool reads = true;
    while(reads && buffer_can_read(buffer)) {
        c = buffer_read(buffer);
        switch(c) {
            case '\r':
                puts_c = false;
                if(!transformation_parser_state->out_ps.first_r_found) {
                    transformation_parser_state->out_ps.first_r_found = true;
                }
                if(transformation_parser_state->out_ps.first_r_found && transformation_parser_state->out_ps.first_n_found && !transformation_parser_state->out_ps.dot_found) {
                        write(fd, "\r\n", 2);
                        ret+=2;
                }
                if(transformation_parser_state->out_ps.dot_found) {
                    transformation_parser_state->out_ps.second_r_found = true;
                }
                break;
            case '\n':
                puts_c = false;
                if(transformation_parser_state->out_ps.first_r_found) {
                    if(!transformation_parser_state->out_ps.first_n_found) {
                        transformation_parser_state->out_ps.first_n_found = true;
                    }
                }
                if(transformation_parser_state->out_ps.second_r_found) {
                    /*Encontramos el final, cierro el fd para mandar EOF */
                    write(fd, "\r\n", 2);
                    ret+=2;
                    close(fd);
                    reads = false;
                }
                break;
            case '.':
                if(transformation_parser_state->out_ps.dot_found) {
                    write(fd, "\r\n", 2);
                    ret+=2;
                    puts_c = true;
                    transformation_parser_state->out_ps.first_r_found = false;
                    transformation_parser_state->out_ps.first_n_found = false;
                    transformation_parser_state->out_ps.dot_found = false;
                }
                if(transformation_parser_state->out_ps.first_n_found) {
                    puts_c = false;
                    transformation_parser_state->out_ps.dot_found = true;
                }
                break;
            default:
                if(transformation_parser_state->out_ps.first_r_found && transformation_parser_state->out_ps.first_n_found && !transformation_parser_state->out_ps.dot_found) {
                    write(fd, "\r\n", 2);
                    ret+=2;
                } else if(transformation_parser_state->out_ps.first_r_found && transformation_parser_state->out_ps.first_n_found && !transformation_parser_state->out_ps.second_r_found) {
                    write(fd, "\r\n", 2);
                    ret+=2;
                }
                puts_c = true;
                transformation_parser_state->out_ps.first_r_found = false;
                transformation_parser_state->out_ps.first_n_found = false;
                transformation_parser_state->out_ps.dot_found = false;
                transformation_parser_state->out_ps.second_r_found = false;
                break;
        }
        if(puts_c) {
            ret++;
            write(fd, &c, 1);
        }
    }
    return ret;
}

ssize_t read_and_parse_transformation_from_fd(int fd, buffer_t buffer, transformation_parser_state_t transformation_parser_state) {
    char c;
    ssize_t n;
    ssize_t ret = 0;
    bool writes = true, puts_dot;

    n = read_from_fd(fd, transformation_parser_state->in_ps.buffer);
    if(n <= 0){
        return n;
    }

    if(buffer_can_write(buffer) && transformation_parser_state->in_ps.last_char != 0) {
        buffer_write(buffer, transformation_parser_state->in_ps.last_char);
        transformation_parser_state->in_ps.last_char = 0;
    }
    while(writes && buffer_can_write(buffer) && buffer_can_read(transformation_parser_state->in_ps.buffer)) {
        c = buffer_read(transformation_parser_state->in_ps.buffer);
        puts_dot = false;
        switch(c) {
            case '\r':
                if(!transformation_parser_state->in_ps.r_found) {
                    transformation_parser_state->in_ps.r_found = true;
                }
                break;
            case '\n':
                if(transformation_parser_state->in_ps.r_found) {
                    if(!transformation_parser_state->in_ps.n_found) {
                        transformation_parser_state->in_ps.n_found = true;
                    }
                }
                break;
            case '.':
                if(transformation_parser_state->in_ps.n_found) {
                    puts_dot = true;
                    transformation_parser_state->in_ps.r_found = false;
                    transformation_parser_state->in_ps.n_found = false;
                }
                break;
            default:
                transformation_parser_state->in_ps.r_found = false;
                transformation_parser_state->in_ps.n_found = false;
                break;
        }
        if(puts_dot) {
            buffer_write(buffer, '.');
            ret++;
        }
        if(writes) {
            if(buffer_can_write(buffer)) {
                buffer_write(buffer, c);
                ret++;
            } else {
                transformation_parser_state->in_ps.last_char = c;
                writes = false;
            }
        }
    }
    return ret;
}


client_parser_state_t init_client_parser_state(void) {
    client_parser_state_t ret = (client_parser_state_t)malloc(sizeof(*ret));
    if(ret == NULL) {
        perror("Error creating parser state for client");
        return NULL;
    }
    reset_client_parser_state(ret);

    return ret;
}

void reset_client_parser_state(client_parser_state_t client_parser_state) {
    client_parser_state->command_len = 0;
    client_parser_state->username_len = 0;
    client_parser_state->found_command = false;
    client_parser_state->waiting_username = false;
}

void free_client_parser_state (client_parser_state_t client_parser_state) {
    free(client_parser_state);
}