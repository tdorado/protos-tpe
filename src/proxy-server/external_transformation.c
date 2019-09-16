#include <stdio.h>
#include <stdlib.h>

#include "include/external_transformation.h"

// HACER FUNCION QUE TOME TEXTO QUE LLEGA DE POP3, HAGA EL PARSEO DE LOS . Y LO MANDE POR UN PIPE 
// A EL STDIN DE UN PROCESO QUE EJECUTE EL CMD QUE ESTA EN PROXY_PARAMS->CMD, Y SE VA A TENER OTRO
// PIPE QUE SE CONECTE CON EL STDOUT DEL PROCESO QUE RECIBE EL TEXTO Y LO VUELVE A PARSEAR CON LOS . EN MODO POP3

char * transform(char * transform_command , char * buffer, int buffer_size) {
    char * message_body = extract_body(email, buffer_size);
    int fd[2];
    pipe(fd);
    if (fork()) {
        close(fd[])
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