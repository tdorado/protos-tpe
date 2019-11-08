#include "include/settings.h"
#include "include/constants.h"

void print_usage() {
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

void print_version() {
    printf("POP3 Proxy Filter %s\n", POP3_FILTER_VERSION);
}

void init_settings(settings_t settings) {
    settings->origin_server_port = DEFAULT_SERVER_PORT;
    settings->management_addr = DEFAULT_MANAGEMENT_ADDR;
    settings->management_port = DEFAULT_MANAGEMENT_PORT;
    settings->local_addr = DEFAULT_LOCAL_ADDR;
    settings->local_port = DEFAULT_LOCAL_PORT;
    settings->replace_message = DEFAULT_REPLACE_MESSAGE;
    settings->media_types = DEFAULT_MEDIA_TYPES;
    settings->cmd = DEFAULT_CMD;
    settings->error_file = DEFAULT_ERROR_FILE;
    settings->transformations = true;
    settings->pipe_lining_supported = false;
}

bool valid_digit(char * digit) {
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

bool valid_address(char * address) {
    return true;
    struct sockaddr_in6 sa;

    if (inet_pton(AF_INET6, address, &(sa.sin6_addr))){
        return true;
    }
    return false;
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

bool valid_media_type(char * media_type){
    int i = 0;

    while(media_type[i] != 0){
        if(media_type[i] == '/'){
            if(media_type[i + 1] != 0){
                return true;
            }
        }
    }

    return false;
}

bool valid_error_file(char * error_file){
    struct stat st;

    if (stat(error_file, &st) < 0)
        return -1;

    return S_ISREG(st.st_mode);
}

int validate_and_set_params(const int argc, char ** argv, settings_t settings) {
    int c;
    bool flag_error = false;

    init_settings(settings);

    while ((c = getopt(argc, argv, "e:l:L:m:M:o:p:P:t:")) != EOF && !flag_error) {
        switch (c) {
            case 'P':
                if (valid_port(optarg)){
                    settings->origin_server_port = (uint16_t) atoi(optarg);
                }
                else{
                    fprintf(stderr, "Invalid -P <origin-server-port> argument. \n");
                    flag_error = true;
                }
                break;
            case 'l':
                if (valid_address(optarg)){
                    settings->local_addr = optarg;
                }
                else{
                    fprintf(stderr, "Invalid -l <local-address> argument. \n");
                    flag_error = true;
                }
                break;
            case 'p':
                if (valid_port(optarg)){
                    settings->local_port = (uint16_t) atoi(optarg);
                }
                else{
                    fprintf(stderr, "Invalid -p <local-port> argument. \n");
                    flag_error = true;
                }
                break;
            case 'L':
                if (valid_address(optarg)){
                    settings->management_addr = optarg;
                }
                else{
                    fprintf(stderr, "Invalid -L <managment-address> argument. \n");
                    flag_error = true;
                }
                break;
            case 'o':
                if (valid_port(optarg)){
                    settings->management_port = (uint16_t) atoi(optarg);
                }
                else{
                    fprintf(stderr, "Invalid -o <managment-port> argument. \n");
                    flag_error = true;
                }
                break;
            case 't':
                settings->cmd = optarg;
                break;
            case 'e':
                if (valid_error_file(optarg)){
                    settings->error_file = optarg;
                }
                else{
                    fprintf(stderr, "Invalid -e <error-file> argument. \n");
                    flag_error = true;
                }
                break;
            case 'm':
                settings->replace_message = optarg;
                break;
            case 'M':
                if (valid_media_type(optarg)){
                    settings->media_types = optarg;
                    settings->transformations = true;
                }
                else{
                    fprintf(stderr, "Invalid -M <filtered-media-type> argument. \n");
                    flag_error = true;
                }
                break;
            default:
                flag_error = true;
                break;
        }
    }

    if (optind == argc - 1 && !flag_error) {
        if (valid_address(argv[optind])) {
            settings->origin_server_addr = argv[optind];
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


int input_parser(const int argc, char ** argv, settings_t settings) {

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

    if (validate_and_set_params(argc, argv, settings) < 0) {
        print_usage();
        return -1;
    }

    return 0;
}
