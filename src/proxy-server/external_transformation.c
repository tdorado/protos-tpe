#include <stdio.h>
#include <stdlib.h>

#include "include/external_transformation.h"

// HACER FUNCION QUE TOME TEXTO QUE LLEGA DE POP3, HAGA EL PARSEO DE LOS . Y LO MANDE POR UN PIPE 
// A EL STDIN DE UN PROCESO QUE EJECUTE EL CMD QUE ESTA EN PROXY_PARAMS->CMD, Y SE VA A TENER OTRO
// PIPE QUE SE CONECTE CON EL STDOUT DEL PROCESO QUE RECIBE EL TEXTO Y LO VUELVE A PARSEAR CON LOS . EN MODO POP3

char * transform(char * transform_command , char * buffer, int buffer_size) {
    char * body = extract_body(buffer, buffer_size);
    char * body = pop3_to_text(buffer, buffer_size);
    int fd[2];
    pipe(fd);
    if (fork()) {
    }
    return transform_command;
}

char * extract_body(char * buffer, int buffer_size) {
    int i=0;
    char * body = malloc(buffer_size * sizeof(char));
    while(email[i] != '\n' && i < buffer_size) {
        body[i] = buffer[i];
        i++;
    }
    if(i == buffer_size) {
        free(body);
        return NULL;
    }
    return body;
}

char * pop3_to_text(char * buffer, int buffer_size) {
    int status = WORKING;
    int actual=0;
    int new=0;
    int on_point = FALSE;
    while(status != FINISHED && actual < buffer_size) {
        switch(buffer[actual]) {
            case '.':
                buffer[new] = buffer[actual];
                new++;
                actual++;
                while(buffer[actual] == '.') {
                    actual++;
                }
                break;
            case '\r':
                if(strcmp(buffer + actual), FINISH_STRING, FINISH_LENGTH) {
                    status = FINISHED;
                    break;
                } else {
                    buffer[new] = buffer[actual];
                    actual++;
                    new++;
                }
            default: 
                buffer[new] = buffer[actual];
                actual++;
                new++;
                break;
        }
    }
    if( actual == buffer_size ) {
        free(buffer);
        return NULL
    }
    return buffer;
}

char * text_to_pop3(char * buffer, int buffer_size) {
}