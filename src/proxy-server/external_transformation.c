#include "include/external_transformation.h"

int external_transformation(input_t proxy_params, char * buffer, int buffer_size) {
    int from, to = buffer_size - FINISH_LENGTH;
    extract_pop3_info(buffer, &from);
    to = call_command(proxy_params, buffer, from, to);
    complete_pop3(buffer, to);
    return to + FINISH_LENGTH;
}

int call_command(input_t proxy_params, char * buffer, int from, int to) {
    int pipeFatherToChild[2];
    int pipeChildToFather[2];
    if ( pipe(pipeFatherToChild) == -1 || pipe(pipeChildToFather) == -1 ){
        return to;
    }

    char * argv[4];
    argv[0] = "bash";
    argv[1] = "-c";
    argv[2] = proxy_params->cmd;
    argv[3] = NULL;

    int pid;
    int i = from;
    int j = from;
    while(i < to) {
        write(pipeFatherToChild[WRITE], buffer + i, 1);
        i++;
    }
    close(pipeFatherToChild[WRITE]);
    if( (pid = fork()) == 0 ) {
        close(STDIN_FILENO);
        dup(pipeFatherToChild[READ]);
        close(STDOUT_FILENO);
        close(pipeChildToFather[READ]);
        dup(pipeChildToFather[WRITE]);

        FILE * f = freopen(proxy_params->error_file, "a+", stderr);
        if(f == NULL){
            exit(EXIT_FAILURE);
        }
        
        int exec_ret = execve("/bin/bash", argv, NULL);

        if (exec_ret == -1){
            fprintf(stderr, "Error on filtered command \n");
        }

    }
    else {
        close(pipeChildToFather[WRITE]);
        buffer[j] = 0;
        while(read(pipeChildToFather[READ], buffer + j, 1) != 0) {
            j++;
        }
        kill(pid, SIGKILL);
    }
    return j;
}

void extract_pop3_info(char * buffer, int * from) {
    int i = 0;
    while(strncmp(buffer + i, "\r\n", 2)) {
        i++;
    }
    *from = i;
}

int pop3_to_text(char * buffer, int buffer_size, char * text) {
    int status = WORKING;
    int actual = 0;
    int new = 0;
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
