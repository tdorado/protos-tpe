#include "include/external_transformation.h"

// HACER FUNCION QUE TOME TEXTO QUE LLEGA DE POP3, HAGA EL PARSEO DE LOS . Y LO MANDE POR UN PIPE
// AL STDIN DE UN PROCESO QUE EJECUTE EL CMD QUE ESTA EN PROXY_PARAMS->CMD, Y SE VA A TENER OTRO
// PIPE QUE SE CONECTE CON EL STDOUT DEL PROCESO QUE RECIBE EL TEXTO Y LO VUELVE A PARSEAR CON LOS . EN MODO POP3

char * external_transformation(char * transform_command , char * buffer, int buffer_size) {
    printf("1");
    char * head = malloc(buffer_size * sizeof(char));
    char * body = malloc(buffer_size * sizeof(char));
    if(extract_pop3_info(buffer, buffer_size, head, body) == -1) {
        free(head);
        free(body);
        return NULL;
    }
    printf("2");
    printf("HEAD: %S, BODY: %S", head, body);
    char * normal_text = malloc(buffer_size * sizeof(char));
    if(pop3_to_text(body, buffer_size, normal_text) == -1) {
        free(head);
        free(body);
        free(normal_text);
        return NULL;
    }
    /*
    int pipe1[2];
    int pipe2[2];
    pipe(pipe1);
    pipe(pipe2);
    printf("Este es el texto normal: %s", normal_text);
    if(fork()) {
        close(0);
        dup(pipe1[0]);
        close(1);
        dup(pipe2[1]);
        execv(transform_command, NULL);
    }
    else {
        int i=0;
        while(normal_text[i] != '\0' && i < buffer_size) {
            write(pipe1[1], normal_text + i, 1);
        }
        int j = 0;
        char * transformed_text = malloc(buffer_size * 2 * sizeof(char));
        while(transformed_text[j] != EOF && transformed_text[j]!='\0') {
            read(pipe2[0], transformed_text + j , 1);
        }
        char * transformed_text_pop3 = malloc(buffer_size * 2 * sizeof(char));
        text_to_pop3(transformed_text, buffer_size * 3, transformed_text_pop3);
        char * rta = malloc( 3 * buffer_size * sizeof(char)); 
        strcat(rta, head);
        strcat(rta, transformed_text_pop3);
    }
    */
}

int extract_pop3_info(char * buffer, int buffer_size, char * head, char * body) {
    int i = 0;
    int j = 0;
    while(strncmp(buffer + i, "\r\n", 2) && i < buffer_size) {
        head[i] = buffer[i];
        i++;
    }
    if(i == buffer_size) {
        return -1;
    }
    i+=2;
    while(strncmp(buffer + i, "\r\n.\r\n", FINISH_LENGTH) && i < buffer_size) {
        body[j] = buffer[i];
        i++;
        j++;
    }
    if( i == buffer_size) {
        return -1;
    }
    return 0;
}

int pop3_to_text(char * buffer, int buffer_size, char * text) {
    int status = WORKING;
    int actual=0;
    int new=0;
    while(status != FINISHED && actual < buffer_size) {
        switch(buffer[actual]) {
            case '.':
                text[new] = buffer[actual];
                new++;
                actual++;
                if(buffer[actual] != '.') {
                    new--;
                }
                break;
            case '\r':
                if(strncmp(buffer + actual, "\r\n.\r\n", FINISH_LENGTH)) {
                    status = FINISHED;
                } else {
                    text[new] = buffer[actual];
                    actual++;
                    new++;
                }
                break;
            default:
                text[new] = buffer[actual];
                actual++;
                new++;
                break;
        }
    }
    if( actual > buffer_size ) {
        free(buffer);
        return -1;
    }
    return 0;
}

int text_to_pop3(char * buffer, int buffer_size, char * pop3_text) {
    int actual = 0;
    int new = 0;
    while(actual < buffer_size) {
        switch(buffer[actual]) {
            case '.':
                pop3_text[new] = buffer[actual];
                new++;
                actual++;
                while(buffer[actual] == '.') {
                    pop3_text[new] = buffer[actual];
                    new++;
                    actual++;
                }
                pop3_text[new] = '.';
                new++;
            default:
                pop3_text[new] = buffer[actual];
                actual++;
                new++;
                break;
        }
    }
    if(actual >= buffer_size) {
        return -1;
    }
    strcpy(pop3_text + new, "\r\n.\r\n");
    return 0;
}
