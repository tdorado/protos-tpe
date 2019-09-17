#include "include/external_transformation.h"
#include <sys/wait.h>


int external_transformation(char * transform_command , char * buffer, int buffer_size) {
    int from, to = buffer_size - FINISH_LENGTH;
    extract_pop3_info(buffer, &from);
    to = call_command(transform_command, buffer, from, to);
    complete_pop3(buffer, to);
    return to + FINISH_LENGTH;
}

int call_command(char * command, char *buffer, int from, int to) {
    int pipe1[2];
    int pipe2[2];
    if ( pipe(pipe1) == -1 || pipe(pipe2) == -1 ){
        return to;
    }
    int pid;
    int i = from;
    int j = from;
    while(i < to) {
        write(pipe1[1], buffer + i, 1);
        i++;
    }
    close(pipe1[1]);
    if( (pid = fork()) == 0 ) {
        close(0);
        dup(pipe1[0]);
        close(1);
        close(pipe2[0]);
        dup(pipe2[1]);
        execl("/bin/cat", command, (char *) 0);
    }
    else {
        close(pipe2[1]);
        buffer[j] = 0;
        while(read(pipe2[0], buffer + j, 1) != 0) {
            j++;
        }
        kill(pid, SIGKILL);
    }
    return j;
}

void extract_pop3_info(char * buffer, int *from) {
    int i = 0;
    while(strncmp(buffer + i, "\r\n", 2)) {
        i++;
    }
    *from = i;
}

int pop3_to_text(char * buffer, int buffer_size, char * text) {
    int status = WORKING;
    int actual=0;
    int new=0;
    int first_point = TRUE;
    while(status != FINISHED && actual < buffer_size) {
        switch(buffer[actual]) {
            case '.':
                if(first_point || buffer[actual+1] == '.') {
                    text[new] = buffer[actual];
                    new++;
                    actual++;
                    first_point = FALSE;
                } else {
                    actual++;
                    first_point = TRUE;
                }
                break;
            case '\0':
                status = FINISHED;
                text[new] = buffer[actual];
                actual++;
                new++;
                break;
            default:
                text[new] = buffer[actual];
                actual++;
                new++;
                break;
        }
    }
    if( actual >= buffer_size ) {
        perror("There was en error passing pop3 body to plain text. It's possible that the format of the pop3 body is corrupted.");
        return -1;
    }
    return 0;
}

int text_to_pop3(char * buffer, int buffer_size, char * pop3_text) {
    int actual = 0;
    int new = 0;
    while(buffer[actual] != '\0' && actual < buffer_size) {
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
        perror("There was en error passing the plain text to pop3 body format. It's possible that the plain text is corrupted.");
        return -1;
    }
    return 0;
}

void complete_pop3(char * buffer, int to) {
    strncat(buffer + to, "\r\n.\r\n", FINISH_LENGTH);
}
