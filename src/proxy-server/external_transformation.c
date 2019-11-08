#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "include/external_transformation.h"
#include "include/error_file.h"

int start_external_transformation_process(settings_t settings, client_t client) {
    int pipeFatherToChild[2] = {-1, -1};
    int pipeChildToFather[2] = {-1, -1};
    
    if ( pipe(pipeFatherToChild) == -1 || pipe(pipeChildToFather) == -1 ){
        perror("Error creating pipes of external transformation process.");
        return -1;
    }

    int flagsFatherToChild = fcntl(pipeFatherToChild[WRITE_END], F_GETFL);
    int flagsChildToFather = fcntl(pipeChildToFather[READ_END], F_GETFL);

    flagsFatherToChild = fcntl(pipeFatherToChild[WRITE_END], F_SETFL, flagsFatherToChild | O_NONBLOCK);
    flagsChildToFather = fcntl(pipeChildToFather[READ_END], F_SETFL, flagsChildToFather | O_NONBLOCK);

    if (flagsFatherToChild == -1 || flagsChildToFather == -1) {
        perror("Error setting external transformation pipes");
        return ERROR_TRANSFORMATION;
    }

    char * argv[4];
    argv[0] = "bash";
    argv[1] = "-c";
    argv[2] = settings->cmd;
    argv[3] = NULL;

    pid_t p_id;
    if((p_id = fork()) == -1) {
        perror("Error creating external transformation process.");
        return ERROR_TRANSFORMATION;
    }
    else if(p_id == 0) {
        close(pipeFatherToChild[WRITE_END]);
        close(pipeChildToFather[READ_END]);
        dup2(pipeFatherToChild[READ_END], STDIN_FILENO);
        dup2(pipeChildToFather[WRITE_END], STDOUT_FILENO);

        redirect_stderr(settings);
        
        int exec_ret = execve("/bin/bash", argv, NULL);

        if (exec_ret == -1){
            fprintf(stderr, "Error on external transformations execve \n");
            exit(EXIT_FAILURE);
        }

    }
    else {
        close(pipeFatherToChild[READ_END]);
        close(pipeChildToFather[WRITE_END]);
        
        client->external_transformation_read_fd = pipeChildToFather[READ_END];
        client->external_transformation_write_fd = pipeFatherToChild[WRITE_END];
    }

    printf("TRANSFORMATION CREADA\n");

    return PROCESS_INITIALIZED;
}