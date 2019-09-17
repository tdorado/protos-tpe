#include "include/external_transformation.h"
#include <sys/wait.h>


char * external_transformation(char * transform_command , char * buffer, int buffer_size) {
    char * head = malloc(buffer_size * sizeof(char));
    char * body = malloc(buffer_size * sizeof(char));
    if(extract_pop3_info(buffer, buffer_size, head, body) == -1) {
        free(head);
        free(body);
        return NULL;
    }
    char * normal_text = malloc(buffer_size * sizeof(char));
    if(pop3_to_text(body, buffer_size, normal_text) == -1) {
        free(head);
        free(body);
        free(normal_text);
        return NULL;
    }
    char * transformed_text = malloc(buffer_size * 2 * sizeof(char));
    call_command(transform_command, normal_text, 100, transformed_text);
    return transformed_text;
}

int call_command(char * command, char * text, int buffer_size, char * transformed_text) {
    int pipe1[2];
    int pipe2[2];
    if ( pipe(pipe1) == -1 || pipe(pipe2) == -1 ){
        return -1;
    }
    int pid;
    int i=0;
    while(text[i] != '\0' && i < buffer_size) {
        write(pipe1[1], text + i, 1);
        i++;
    }
    close(pipe1[1]);
    if(pid = fork()) {
        close(0);
        dup(pipe1[0]);
        close(1);
        close(pipe2[0]);
        dup(pipe2[1]);
        execl("/bin/cat", command, (char *) 0);
    }
    else {
        close(pipe2[1]);
        int j = 0;
        transformed_text[j] = 0;
        while(read(pipe2[0], transformed_text + j, 1) != 0) {
            j++;
        }
    }
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
