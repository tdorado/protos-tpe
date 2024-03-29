#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "include/settings.h"

char ** init_envs(void);
void free_envs(char ** envs);

void print_usage(void) {
    printf("USAGE: ./pop3filter [POSIX style options] <origin-server-address> \n"
            "   <origin-server-address>        Address of POP3 origin server. \n\n"
            "POSIX style options: \n"
            "   -h                             Prints out help and ends. \n"
            "   -v                             Prints out the POP3 Proxy Filter version and ends. \n"
            "   -P <origin-server-port>        Specifies TCP port where the POP3 server is located. By default is 110. \n"
            "   -l <listen-address>            Specifies the address where the proxy_pop3 will serve. By default is any. \n"
            "   -p <listen-port>               Specifies TCP port where to listen for incoming POP3 connections. By default is 1110. \n"
            "   -L <management-address>        Specifies the address where the management service will serve. By default is loopback.\n"
            "   -o <management-port>           Specifies SCTP port where the management server is located. By default is 9090. \n"
            "   -M <filtered-media-type>       Specifies a media types to be censored. By default applies no filters. \n"
            "   -m <replacement-message>       Specifies the message to replace filtered text. By default is \"Parte Reemplazada.\". \n"
            "   -t <filtered-command>          Command used for external transformations. By default applies no transformations. \n"
            "   -e <error-file>                Specifies the file where to redirect stderr. By default is /dev/null'. \n");
}

void print_version(void) {
    printf("POP3 Proxy Filter %s\n", POP3_FILTER_VERSION);
}

bool valid_digit(char * digit) {
    while (*digit) {
        if (*digit >= '0' && *digit <= '9') {
            digit++;
        } else {
            return false;
        }
    }
    return true;
}

bool valid_address(char * address) {
    struct sockaddr_in6 sa6;
    struct sockaddr_in sa;
    if (strcmp(address, "loopback") == 0 || strcmp(address, "any") == 0) {
        return true;
    } else {
        return inet_pton(AF_INET6, address, &(sa6.sin6_addr)) == 1 || inet_pton(AF_INET, address, &(sa.sin_addr)) == 1;
    }
}

bool valid_port(char * port) {
    int num;

    if (!valid_digit(port)) {
        return false;
    }

    num = atoi(port);

    if (num >= 0 && num <= 65535) {
        return true;
    }

    return false;
}

bool valid_media_type(char * media_type) {
    int i = 0;

    while(media_type[i] != 0) {
        if(media_type[i] == '/') {
            if(media_type[i + 1] != 0) {
                return true;
            }
        }
    }

    return false;
}

bool valid_error_file(char * error_file) {
    struct stat st;

    if (stat(error_file, &st) < 0)
        return -1;

    return S_ISREG(st.st_mode);
}

int validate_and_set_settings(const int argc, char ** argv, settings_t settings) {
    int c;
    bool flag_error = false;

    while ((c = getopt(argc, argv, "e:l:L:m:M:o:p:P:t:")) != EOF && !flag_error) {
        switch (c) {
            case 'P':
                if (valid_port(optarg)) {
                    settings->origin_server_port = (uint16_t) atoi(optarg);
                } else {
                    perror("Invalid -P <origin-server-port> argument. \n");
                    flag_error = true;
                }
                break;
            case 'l':
                if (valid_address(optarg)) {
                    settings->local_addr = optarg;
                } else {
                    perror("Invalid -l <local-address> argument. \n");
                    flag_error = true;
                }
                break;
            case 'p':
                if (valid_port(optarg)) {
                    settings->local_port = (uint16_t) atoi(optarg);
                } else {
                    perror("Invalid -p <local-port> argument. \n");
                    flag_error = true;
                }
                break;
            case 'L':
                if (valid_address(optarg)) {
                    settings->management_addr = optarg;
                } else {
                    perror("Invalid -L <managment-address> argument. \n");
                    flag_error = true;
                }
                break;
            case 'o':
                if (valid_port(optarg)) {
                    settings->management_port = (uint16_t) atoi(optarg);
                } else {
                    perror("Invalid -o <managment-port> argument. \n");
                    flag_error = true;
                }
                break;
            case 't':
                strcpy(settings->cmd, (char *)optarg);
                if(strstr(settings->cmd, ".") != NULL){
                    if(strstr(settings->cmd, "bash") == NULL){
                        if(settings->cmd[0] != '.'){
                            strcpy(settings->cmd + 2, settings->cmd);
                            settings->cmd[0] = '.';
                            settings->cmd[1] = '/';
                        }
                    }
                }
                settings->cmd_transformations = true;
                break;
            case 'e':
                if (valid_error_file(optarg)) {
                    settings->error_file = optarg;
                } else {
                    perror("Invalid -e <error-file> argument. \n");
                    flag_error = true;
                }
                break;
            case 'm':
                settings->replace_message = optarg;
                break;
            case 'M':
                    strcpy(settings->media_types, optarg);
                    settings->mtype_transformations = true;
                break;
            default:
                flag_error = true;
                break;
        }
    }

    if (optind == argc - 1 && !flag_error) {
        settings->origin_server_addr = argv[optind];
    } else if (!flag_error) {
        perror("Expected <origin-server-address> argument after options. \n");
        flag_error = true;
    }

    if (flag_error) {
        return -1;
    }

    return 0;
}


int input_parser(const int argc, char ** argv, settings_t settings) {

    if (argc < 2) {
        perror("Expected one argument at least to execute. \n");
        print_usage();
        return -1;
    }

    if (strcmp(argv[1], "-h") == 0) {
        print_usage();
        return -1;
    }

    if (strcmp(argv[1], "-v") == 0) {
        print_version();
        return -1;
    }

    if (validate_and_set_settings(argc, argv, settings) < 0) {
        print_usage();
        return -1;
    }

    return 0;
}

void free_settings(settings_t settings) {
    free(settings->media_types);
    free(settings->cmd);
    free_envs(settings->envs_for_transformation);
    free(settings->cmd_for_transformation);
    free(settings);
}

settings_t init_settings(void) {
    settings_t ret = (settings_t)malloc(sizeof(*ret));
    if(ret == NULL) {
        perror("Error creating settings");
        exit(EXIT_FAILURE);
    }

    ret->origin_server_port = DEFAULT_SERVER_PORT;
    ret->management_addr = DEFAULT_MANAGEMENT_ADDR;
    ret->management_port = DEFAULT_MANAGEMENT_PORT;
    ret->local_addr = DEFAULT_LOCAL_ADDR;
    ret->local_port = DEFAULT_LOCAL_PORT;
    ret->replace_message = DEFAULT_REPLACE_MESSAGE;
    ret->media_types = (char *)malloc(CMD_BUFFER);
    if(ret->media_types == NULL){
        perror("Error creating settings");
        exit(EXIT_FAILURE);
    }
    ret->media_types[0] = 0;
    ret->cmd = (char *)malloc(CMD_BUFFER);
    if(ret->cmd == NULL){
        perror("Error creating settings");
        exit(EXIT_FAILURE);
    }
    strcpy(ret->cmd, DEFAULT_CMD);
    ret->error_file = DEFAULT_ERROR_FILE;
    ret->cmd_transformations = false;
    ret->mtype_transformations = false;
    ret->version = POP3_FILTER_VERSION;
    ret->pipelining = false;
    ret->envs_for_transformation = init_envs();
    if(ret->envs_for_transformation == NULL){
        perror("Error creating settings");
        exit(EXIT_FAILURE);
    }
    ret->cmd_for_transformation = malloc(CMD_BUFFER);
    if(ret->cmd_for_transformation == NULL){
        perror("Error creating settings");
        exit(EXIT_FAILURE);
    }

    return ret;
}

char ** init_envs(void){
    char ** envs = malloc(6 * sizeof(char *));
    if(envs == NULL){
        return NULL;
    }
    
    for(int i = 0; i < 5; i++){
        envs[i] = malloc(ENV_BUFFER);
        if(envs[i] == NULL){
            for(int j = i - 1; j >= 0; j--){
                free(envs[j]);
            }
            return NULL;
        }
    }

    return envs;
}
void free_envs(char ** envs){
    free(envs[0]);
    free(envs[1]);
    free(envs[2]);
    free(envs[3]);
    free(envs[4]);
    free(envs);
}