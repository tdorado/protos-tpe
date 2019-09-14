#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

#include "include/input_parser.h"
#include "include/constants.h"

#define DELIM "."

void print_usage() {
    printf("USAGE: ./pop3filter [POSIX style options] <origin-server-address> \n"
            "   <origin-server-address>        Address of POP3 origin server. \n\n"
            "POSIX style options: \n"
            "   -h                             Prints out help and ends. \n"
            "   -v                             Prints out the POP3 Proxy Filter version and ends. \n"
            "   -P <origin-server-port>        Specifies TCP port where the POP3 server is located. By default is 110. \n"
            "   -l <listen-address>            Specifies the address where the proxy_pop3 will serve. \n"
            "   -p <listen-port>               Specifies TCP port where to listen for incoming POP3 connections. By default is 1110. \n"
            "   -L <management-address>        Specifies the address where the management service will serve. \n"
            "   -o <management-port>           Specifies SCTP port where the management server is located. By default is 9090. \n"
            "   -M <filtered-media-type>       Specifies a media types to be censored. \n"
            "   -m <replacement-message>       Specifies the message to replace filtered text. \n"
            "   -t <filtered-command>          Command used for external transformations. By default applies no transformations. \n"
            "   -e <error-file>                Specifies the file where to redirect stderr. By default is /dev/null'. \n");
}

void print_version() {
    printf("POP3 Proxy Filter %s\n", POP3_FILTER_VERSION);
}

void initialize_values(input_t proxy_params) {
    proxy_params->origin_server_port = DEFAULT_SERVER_PORT;
    proxy_params->management_addr = DEFAULT_MANAGEMENT_ADDR;
    proxy_params->management_port = DEFAULT_MANAGEMENT_PORT;
    proxy_params->local_addr = DEFAULT_LOCAL_ADDR;
    proxy_params->local_port = DEFAULT_LOCAL_PORT;
    proxy_params->replace_message = DEFAULT_REPLACE_MESSAGE;
    proxy_params->media_types = DEFAULT_MEDIA_TYPES;
    proxy_params->cmd = DEFAULT_CMD;
    proxy_params->error_file = DEFAULT_ERROR_FILE;
}

bool valid_digit(char *digit) {
    while (*digit) {
        if (*digit >= '0' && *digit <= '9') {
            digit++;
        }
        else {
            return false;
        }
    }
    return true;
}
 
bool valid_address(char *address) {
    int num, dots = 0;
    char *ptr;
 
    if (address == NULL) {
        return false;
    }
 
    ptr = strtok(address, DELIM);
 
    if (ptr == NULL) {
        return false;
    }
 
    while (ptr) {
 
        if (!valid_digit(ptr)) {
            return false;
        }
 
        num = atoi(ptr);
 
        if (num >= 0 && num <= 255) {
            ptr = strtok(NULL, DELIM);
            if (ptr != NULL) {
                dots++;
            }
        }
        else {
            return false;
        }
    }
 
    if (dots != 3) {
        return false;
    }

    return true;
}

bool valid_port(char *port) {
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

int validate_and_set_params(const int argc, char ** argv, input_t proxy_params) {
    int c;
    bool flag_error = false;
    
    initialize_values(proxy_params);

    while ((c = getopt(argc, argv, "e:l:L:m:M:o:p:P:t:")) != EOF && !flag_error) {
        switch (c) {
            case 'P':
                if (valid_port(optarg)){
                    proxy_params->origin_server_port = (uint16_t) atoi(optarg);
                }
                else{
                    fprintf(stderr, "Invalid -P <origin-server-port> argument. \n");
                    flag_error = true;
                }
                break;
            case 'l':
                if (valid_address(optarg)){
                    proxy_params->local_addr = optarg;
                }
                else{
                    fprintf(stderr, "Invalid -l <local-address> argument. \n");
                    flag_error = true;
                }
                break;
            case 'p':
                if (valid_port(optarg)){
                    proxy_params->local_port = (uint16_t) atoi(optarg);
                }
                else{
                    fprintf(stderr, "Invalid -p <local-port> argument. \n");
                    flag_error = true;
                }
                break;
            case 'L':
                if (valid_address(optarg)){
                    proxy_params->management_addr = optarg;
                }
                else{
                    fprintf(stderr, "Invalid -L <managment-address> argument. \n");
                    flag_error = true;
                }
                break;
            case 'o':
                if (valid_port(optarg)){
                    proxy_params->management_port = (uint16_t) atoi(optarg);
                }
                else{
                    fprintf(stderr, "Invalid -o <managment-port> argument. \n");
                    flag_error = true;
                }
                break;
            case 't':
                proxy_params->cmd = optarg;
                break;
            case 'e':
                proxy_params->error_file = optarg;
                break;
            case 'm':
                proxy_params->replace_message = optarg;
                break;
            case 'M':
                proxy_params->media_types = optarg;
                break;
            default:
                flag_error = true;
                break;
        }
    } 

    if (optind == argc - 1 && !flag_error) {
        if (valid_address(argv[optind])) {
            proxy_params->origin_server_addr = argv[optind]; 
        }
        else{
            fprintf(stderr, "Invalid <origin-server-address> argument. \n");
            flag_error = true;
        }
    }
    else if (!flag_error) {
        fprintf(stderr, "Expected <origin-server-address> argument after options. \n");
        flag_error = true;
    }

    if (flag_error) {
        return -1;
    }

    return 0;
}


int input_parser(const int argc, char ** argv, input_t proxy_params) {

    if (argc < 2) {
        fprintf(stderr, "Expected one argument at least to execute. \n");
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

    if (validate_and_set_params(argc, argv, proxy_params) < 0) {
        print_usage();
        return -1;
    }

    return 0;
}