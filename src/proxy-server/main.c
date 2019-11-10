#include "include/constants.h"

#include "include/proxy_socket.h"
#include "include/settings.h"
#include "include/logs.h"
#include "include/metrics.h"
#include "include/proxy_clients.h"
#include "include/error_file.h"

void init_server_config(int argc, char ** argv);
int start_server();
bool set_fds(int * max_fd, fd_set * read_fds, fd_set * write_fds, client_list_t client_list, settings_t settings, metrics_t metrics);
void resolve_connections(int proxy_fd, struct sockaddr_in6 server_addr, socklen_t * server_addr_len, int admin_fd, fd_set * read_fds, fd_set * write_fds, client_list_t client_list, settings_t settings, metrics_t metrics);
static void signal_action_handler(const int signal_number);
static void thread_handler(const int signal_number);
int set_up_signals();
void turn_off_proxy();

static settings_t settings;
static metrics_t metrics;
static client_list_t client_list;
static int proxy_fd;
static int admin_fd;

int main(int argc, char ** argv) {

    init_server_config(argc, argv);

    return start_server();
}

void init_server_config(int argc, char ** argv){
    
    if (set_up_signals() != 0) {
        exit(EXIT_FAILURE);
    }

    settings = init_settings();
    
    if (input_parser(argc, argv, settings) < 0) {
        exit(EXIT_FAILURE);
    }

    redirect_stderr(settings);

    metrics = init_metrics();
    client_list = init_client_list();

}

int start_server(){
    
    struct sockaddr_in6 server_addr;
    socklen_t server_addr_len;
    //struct sockaddr_in admin_addr;
    //socklen_t admin_addr_len;

    //memset(&admin_addr, 0, sizeof(admin_addr));

    proxy_fd = init_proxy_socket(&server_addr, &server_addr_len, settings);
    //admin_fd = init_admin_socket(&admin_addr, &admin_addr_len, settings);

    sigset_t blockset;
    struct sigaction sa;
    sa.sa_handler = thread_handler;
    sigemptyset(&blockset);
    sigaddset(&blockset, SIGUSR1);
    sigaddset(&blockset, SIGINT);
    sigprocmask(SIG_BLOCK, &blockset, NULL);
    if (sigaction(SIGUSR1, &sa, NULL) != 0) {
        perror("Error setting signal action");
        turn_off_proxy();
        return 1;
    }

    int max_fd, res;
    fd_set read_fds, write_fds;
    sigset_t emptyset;
    sigemptyset(&emptyset);

    while(true){
        if(!set_fds(&max_fd, &read_fds, &write_fds, client_list, settings, metrics)){
            res = pselect(max_fd + 1, &read_fds, &write_fds, NULL, NULL, &emptyset);

            if(res == -1 && errno != EINTR){
                perror("Error on pselect");
                exit(EXIT_FAILURE);
            }
            else{
                errno = 0;
            }
            
            resolve_connections(proxy_fd, server_addr, &server_addr_len, admin_fd, &read_fds, &write_fds, client_list, settings, metrics);
        }
    }

    return 1;
}

bool set_fds(int * max_fd, fd_set * read_fds, fd_set * write_fds, client_list_t client_list, settings_t settings, metrics_t metrics){
        
    FD_ZERO(read_fds);
    FD_ZERO(write_fds);

    *max_fd = 0;

    set_proxy_fd(proxy_fd, max_fd, read_fds);
    //set_admin_fd(admin_fd, max_fd, read_fds);

    client_t client = client_list->first;

    while(client != NULL){
        if(set_client_fds(client, client_list, max_fd, read_fds, write_fds, settings, metrics) == -1){
            return true;
        }
        client = client->next;
    }

    return false;
}

void resolve_connections(int proxy_fd, struct sockaddr_in6 server_addr, socklen_t * server_addr_len, int admin_fd, fd_set * read_fds, fd_set * write_fds, client_list_t client_list, settings_t settings, metrics_t metrics){
    
    resolve_proxy_client(proxy_fd, read_fds, client_list, server_addr, server_addr_len, settings, metrics);
    //resolve_admin_client(admin_fd, read_fds, &admin_addr, admin_addr_len, settings, metrics);

    client_t client = client_list->first;
    while (client != NULL) {
        resolve_client(client, client_list, read_fds, write_fds, settings, metrics);
        client = client->next;
    }
}

static void signal_action_handler(const int signal_number) {
    switch(signal_number){
        case SIGINT:
            printf("\nSIGINT catched.\n");
            turn_off_proxy();
            break;
        case SIGSEGV:
            printf("\nSIGSEGV catched.\n");
            turn_off_proxy();
            break;
        case SIGTERM:
            printf("\nSIGTERM catched.\n");
            turn_off_proxy();
            break;
        case SIGPIPE:
            printf("\nSIGPIPE catched.\n");
            turn_off_proxy();
            break;
    }
}

static void thread_handler(const int signal_number) {
    
}

int set_up_signals(void) {
    struct sigaction sa;
    sa.sa_handler = signal_action_handler;
    if (sigaction(SIGINT, &sa, NULL) != 0) {
        perror("Error setting signal action");
        turn_off_proxy();
        return -1;
    }
    if (sigaction(SIGPIPE, &sa, NULL) != 0) {
        perror("Error setting signal action");
        turn_off_proxy();
        return -1;
    }
    if (sigaction(SIGTERM, &sa, NULL) != 0) {
        perror("Error setting signal action");
        turn_off_proxy();
        return -1;
    }
    if (sigaction(SIGSEGV, &sa, NULL) != 0) {
        perror("Error setting signal action");
        turn_off_proxy();
        return -1;
    }
    return 0;
}

void turn_off_proxy() {
    free_client_list(client_list);
    free_settings(settings);
    free_metrics(metrics);
    //close(admin_fd);
    close(proxy_fd);

    exit(EXIT_FAILURE);
}