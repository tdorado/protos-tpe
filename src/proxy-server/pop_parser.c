#include <stdlib.h>
#include <errno.h>

#include "include/pop_parser.h"
#include "include/proxy_clients.h"

parser_state_t init_parser_state() {
    parser_state_t ret = (parser_state_t)malloc(sizeof(*ret));
    if(ret == NULL) {
        perror("Error creating parser state for client");
        return NULL;
    }

    reset_parser_state(ret);

    return ret;
}

void reset_parser_state(parser_state_t parser_state) {
    parser_state->out_ps.first_r_found = false;
    parser_state->out_ps.first_n_found = false;
    parser_state->out_ps.dot_found = false;
    parser_state->out_ps.second_r_found = false;

    parser_state->in_ps.r_found = false;
    parser_state->in_ps.n_found = false;
    parser_state->in_ps.last_char = 0;
}

void free_parser_state (parser_state_t parser_state) {
    free(parser_state);
}

ssize_t write_and_parse_to_fd(int fd, buffer_t buffer, parser_state_t parser_state) {
    char c;
    ssize_t ret = 0;
    bool puts_c = true;
    bool reads = true;
    while(reads && buffer_can_read(buffer)) {
        c = buffer_read(buffer);
        switch(c) {
            case '\r':
                puts_c = false;
                if(!parser_state->out_ps.first_r_found) {
                    parser_state->out_ps.first_r_found = true;
                }
                if(parser_state->out_ps.first_r_found && parser_state->out_ps.first_n_found && !parser_state->out_ps.dot_found) {
                        write(fd, "\r\n", 2);
                        ret+=2;
                }
                if(parser_state->out_ps.dot_found) {
                    parser_state->out_ps.second_r_found = true;
                }
                break;
            case '\n':
                puts_c = false;
                if(parser_state->out_ps.first_r_found) {
                    if(!parser_state->out_ps.first_n_found) {
                        parser_state->out_ps.first_n_found = true;
                    }
                }
                if(parser_state->out_ps.second_r_found) {
                    //\r\n.\r\n found
                    char aux = EOF;
                    write(fd, &aux, 1);
                    reads = false;
                }
                break;
            case '.':
                if(parser_state->out_ps.dot_found) {
                    write(fd, "\r\n", 2);
                    ret+=2;
                    puts_c = true;
                    parser_state->out_ps.first_r_found = false;
                    parser_state->out_ps.first_n_found = false;
                    parser_state->out_ps.dot_found = false;
                }
                if(parser_state->out_ps.first_n_found) {
                    puts_c = false;
                    parser_state->out_ps.dot_found = true;
                }
                break;
            default:
                if(parser_state->out_ps.first_r_found && parser_state->out_ps.first_n_found && !parser_state->out_ps.dot_found) {
                    write(fd, "\r\n", 2);
                    ret+=2;
                } else if(parser_state->out_ps.first_r_found && parser_state->out_ps.first_n_found && !parser_state->out_ps.second_r_found) {
                    write(fd, "\r\n", 2);
                    ret+=2;
                }
                puts_c = true;
                parser_state->out_ps.first_r_found = false;
                parser_state->out_ps.first_n_found = false;
                parser_state->out_ps.dot_found = false;
                parser_state->out_ps.second_r_found = false;
                break;
        }
        if(puts_c) {
            ret++;
            write(fd, &c, 1);
        }
    }
    return ret;
}

ssize_t read_and_parse_from_fd(int fd, buffer_t buffer, parser_state_t parser_state) {
    char c;
    ssize_t n;
    ssize_t ret = 0;
    bool writes = true, puts_dot;
    if(buffer_can_write(buffer) && parser_state->in_ps.last_char != 0) {
        buffer_write(buffer, parser_state->in_ps.last_char);
        parser_state->in_ps.last_char = 0;
    }
    while(writes && buffer_can_write(buffer) && ((n = read(fd, &c, 1) != -1))) {
        if(errno != 0) {
            return ret;
        }
        puts_dot = false;
        switch(c) {
            case '\r':
                if(!parser_state->in_ps.r_found) {
                    parser_state->in_ps.r_found = true;
                }
                break;
            case '\n':
                if(parser_state->in_ps.r_found) {
                    if(!parser_state->in_ps.n_found) {
                        parser_state->in_ps.n_found = true;
                    }
                }
                break;
            case '.':
                if(parser_state->in_ps.n_found) {
                    puts_dot = true;
                    parser_state->in_ps.r_found = false;
                    parser_state->in_ps.n_found = false;
                }
                break;
            case EOF:
                writes = false;
                break;
            default:
                parser_state->in_ps.r_found = false;
                parser_state->in_ps.n_found = false;
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
                parser_state->in_ps.last_char = c;
                writes = false;
            }
        }
    }
    return ret;
}
