#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "include/external_transformation.h"
#include "include/error_file.h"

#define READ_END 0
#define WRITE_END 1

void set_envs(char ** envs, settings_t settings, client_t client);

int start_external_transformation_process(settings_t settings, client_t client) {
    int pipeFatherToChild[2] = {-1, -1};
    int pipeChildToFather[2] = {-1, -1};

    if ( pipe(pipeFatherToChild) == -1 || pipe(pipeChildToFather) == -1 ) {
        perror("Error creating pipes of external transformation process.");
        return -1;
    }

    int flagsFatherToChild = fcntl(pipeFatherToChild[WRITE_END], F_GETFL);
    int flagsChildToFather = fcntl(pipeChildToFather[READ_END], F_GETFL);

    flagsFatherToChild = fcntl(pipeFatherToChild[WRITE_END], F_SETFL, flagsFatherToChild | O_NONBLOCK);
    flagsChildToFather = fcntl(pipeChildToFather[READ_END], F_SETFL, flagsChildToFather | O_NONBLOCK);

    if (flagsFatherToChild == -1 || flagsChildToFather == -1) {
        perror("Error setting external transformation pipes");
        return ERROR_TRANSFORMATION_PROCESS;
    }

    char * argv[4];
    argv[0] = "bash";
    argv[1] = "-c";
    argv[3] = NULL;

    set_envs(settings->envs_for_transformation, settings, client);
    if(settings->mtype_transformations && settings->cmd_transformations) {
        // Primero stripmime y despues el cmd
        sprintf(settings->cmd_for_transformation, "./stripmime | %s", settings->cmd);
        argv[2] = settings->cmd_for_transformation;
    } else if (settings->mtype_transformations){
        // Solo stripmime
        argv[2] = "./stripmime";
    } else {
        // Solo cmd
        argv[2] = settings->cmd;
    }

    pid_t p_id;
    if((p_id = fork()) == -1) {
        perror("Error creating external transformation process.");
        return ERROR_TRANSFORMATION_PROCESS;
    } else if(p_id == 0) {
        close(pipeFatherToChild[WRITE_END]);
        close(pipeChildToFather[READ_END]);
        dup2(pipeFatherToChild[READ_END], STDIN_FILENO);
        dup2(pipeChildToFather[WRITE_END], STDOUT_FILENO);

        redirect_stderr(settings);

        int exec_ret = execve("/bin/bash", argv, settings->envs_for_transformation);

        if (exec_ret == -1) {
            perror("Error on external transformations execve \n");
            exit(EXIT_FAILURE);
        }

    } else {
        close(pipeFatherToChild[READ_END]);
        close(pipeChildToFather[WRITE_END]);

        client->external_transformation_read_fd = pipeChildToFather[READ_END];
        client->external_transformation_write_fd = pipeFatherToChild[WRITE_END];
    }

    return PROCESS_INITIALIZED;
}

void set_envs(char ** envs, settings_t settings, client_t client){
    sprintf(envs[0], "FILTER_MEDIAS=%s", settings->media_types);
    sprintf(envs[1], "FILTER_MSG=%s", settings->replace_message);
    sprintf(envs[2], "POP3FILTER_VERSION=%s", settings->version);
    sprintf(envs[3], "POP3_USERNAME=%s", client->username);
    sprintf(envs[4], "POP3_SERVER=%s", settings->origin_server_addr);
    envs[5] = 0;
}