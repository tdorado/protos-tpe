#include "include/admin_socket.h"
#include <stdlib.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/sctp.h>
#include <sys/select.h>

#include "include/admin_socket.h"

/**                                            --
 * FALTA LOGICA PARA AUTENTICACION              |   PROBLEMA DE TOMAS DEL DOMINGO
 * COMO CARAJO MANEJAMOS SET Y RM?              |       quizás tenga que tocar las settings?
 * PODRIA EXTRAER LOS REQ,RESP, OPREATIONS      | --> me gusta la idea, commons?
 *                                             --
*/

#define BUFFER_MAX 2048
#define MAX_CONCURRENT_CONECTIONS 10
#define MAX_INT_DIGITS 10

int create_sctp_socket() {
    int admin_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
    if (admin_fd == -1)
    {
        printf("Failed to create sctp socket\n");
        perror("socket()");
        exit(1);
    }
    return admin_fd;
}

int binding(int admin_socket, struct sockaddr_in *server_addr, size_t server_addr_size) {
    int binding_fd = bind(admin_socket, server_addr, server_addr_size);
    if (binding_fd == -1)
    {
        printf("Bind failed \n");
        perror("bind()");
        close(admin_socket);
        exit(1);
    }
    return binding_fd;
}

int set_socket_opt(int admin_socket, int level, int opt_name, void *opt_val, socklen_t opt_len) {
    int set_socket_fd = setsockopt(admin_socket, level, opt_name, opt_val, opt_len);
    if (set_socket_fd == -1)
    {
        printf("setsockopt() failed \n");
        perror("setsockopt()");
        close(admin_socket);
        exit(1);
    }
    return set_socket_fd;
}

int start_listen(int fd, int backlog) {
    int listen_fd = listen(fd, 5);
    if (listen_fd == -1)
    {
        printf("listen() failed \n");
        perror("listen()");
        close(fd);
        exit(1);
    }
    return listen_fd;
}

int init_admin_socket(struct sockaddr_in *server_addr, socklen_t *server_addr_len, settings_t settings) {
    int admin_socket, ret;
    struct sockaddr_in serv_addr;
    struct sctp_initmsg init_msg;

    admin_socket = create_sctp_socket(); 

    bzero((void *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(settings->management_port);

    ret = binding(admin_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    /* Specify that a maximum of 10 streams will be available per socket */
    memset(&init_msg, 0, sizeof(init_msg));
    init_msg.sinit_num_ostreams = MAX_CONCURRENT_CONECTIONS;
    init_msg.sinit_max_instreams = MAX_CONCURRENT_CONECTIONS;
    init_msg.sinit_max_attempts = 5;
    ret = set_socket_opt(admin_socket, IPPROTO_SCTP, SCTP_INITMSG, &init_msg, sizeof(init_msg));

    ret = start_listen(admin_socket, MAX_CONCURRENT_CONECTIONS); 

    return admin_socket;
}

void resolve_admin_client(int admin_socket, fd_set *readFDs, struct sockaddr_in *admin_addr, socklen_t *admin_addr_len, settings_t settings, metrics_t metrics) {
    if (FD_ISSET(admin_socket, readFDs))
    {
        resolve_sctp_client(admin_socket, admin_addr, admin_addr_len, settings, metrics);
    }
}

void set_admin_fd(const int admin_fd, int *max_fd, fd_set *read_fds){
    FD_SET(admin_fd, read_fds);
    if (admin_fd > *max_fd) {
        *max_fd = admin_fd;
    }
}

void resolve_sctp_client(int admin_socket, struct sockaddr_in *admin_addr, int admin_addr_len, settings_t settings, metrics_t metrics) {
    int flags = 0;
    __uint8_t admin_received_msg[BUFFER_MAX];
    char *proxy_response_msg = malloc(BUFFER_MAX * sizeof(char));

    struct sctp_sndrcvinfo sndrcv_info;
    int ret;

    int msg_reveived_len;

    bool auth = false;

    int connSock = accept(admin_socket, (struct sockaddr *)NULL, (unsigned int *)NULL);

    msg_reveived_len = sctp_recvmsg(connSock, admin_received_msg, BUFFER_MAX, (struct sockaddr *)NULL, 0, &sndrcv_info, &flags);
    printf("A punto de entrar? \n");
    if (msg_reveived_len == -1) {
        printf("Error in sctp_recvmsg()\n");
        perror("sctp_recvmsg()");
    } else {
        printf("Recibio algo\n");
        printf("Esta autenticado? %d\n", auth);
        if (!auth && (admin_received_msg[0] == LOGIN_REQUEST || admin_received_msg[0] == LOGOUT_REQUEST)) {
            auth = parseLoginOrLogout(admin_received_msg);
            if (auth) {
                strcpy(proxy_response_msg, "Login succesful");
            } else if (admin_received_msg[0] == LOGOUT_REQUEST) {
                strcpy(proxy_response_msg, "Logout succesful");
            } else {
                strcpy(proxy_response_msg, "Forbidden");
            }
        } else if (auth) {
            proxy_response_msg = parse_request(admin_received_msg, settings, metrics);
        } else {
            strcpy(proxy_response_msg, "Forbidden");
        }
    }

    if (msg_reveived_len != 0) {
        ret = sctp_sendmsg(connSock, (void *)proxy_response_msg, (size_t)BUFFER_MAX, NULL, 0, 0, 0, 0, 0, 0);
        if (ret == -1) {
            printf("Error in sctp_sendmsg()\n");
            perror("sctp_sendmsg()");
        }
        else {
            printf("Successfully sent %d bytes data to admin\n", ret);
        }
        free(proxy_response_msg);
    }
}

bool parseLoginOrLogout(__uint8_t *admin_received_msg) {
    switch (admin_received_msg[0]) {
        case LOGIN_REQUEST:
            return isTokenValid(admin_received_msg+1);
        case LOGOUT_REQUEST:
            return false;
        default:
            return false;
    }
}

bool isTokenValid(__uint8_t *token) {
    return strcmp((char *)token, "tokenazo") == 0 ? true : false;
}

char *parse_request(__uint8_t *admin_received_msg, settings_t settings, metrics_t metrics) {
    char *response = NULL;
    switch(admin_received_msg[0]) {
        case GET_REQUEST:
            response = parse_get(admin_received_msg + 1, settings, metrics);
            return response;
        case SET_REQUEST:
            response = parse_set(admin_received_msg + 1, settings, metrics);
            return response;
        case RM_REQUEST:
            response = parse_rm(admin_received_msg + 1, settings, metrics);
            return response;
    }
    return response;
}

char *parse_get(__uint8_t *admin_received_msg, settings_t settings, metrics_t metrics) {
    char *response = NULL;
    switch(admin_received_msg[0]) {
        case CONCURRENT:
            response = buildMetricsResponse(metrics->concurrent_connections, "Concurrent conections: ");
            return response;
        case ACCESSES:
            response = buildMetricsResponse(metrics->total_connections, "Total connections: ");
            return response;
        case BYTES:
            response = buildMetricsResponse(metrics->bytes_transfered, "Bytes transfered: ");
            return response;
        case CMD:
            response = buildGetResponseWithString(settings->cmd, "Command active: ");
            return response;
        case MTYPES:
            response = buildGetResponseWithString(settings->media_types, "Media types actives: ");
            return response;
    }
    return response;
}

char *buildMetricsResponse(int data, char *initial_msg) {
    char *response = malloc(strlen(initial_msg) + MAX_INT_DIGITS + 1);
    strncpy(response, initial_msg, strlen(initial_msg));
    sprintf(response + strlen(initial_msg), "%d\0", data);
    return response;
}

char *buildGetResponseWithString(char *data, char *initial_msg) {
    char *response = malloc(strlen(initial_msg) + strlen(data) + 1);
    strncpy(response, initial_msg, strlen(initial_msg));
    strcpy(response + strlen(initial_msg), data);
    return response;
}


char *parse_set(__uint8_t *admin_received_msg, settings_t settings, metrics_t metrics) {
    char *response = "TODO SET";
    switch(admin_received_msg[0]) {
        case CMD:
            break;
        case MTYPES:
            break;
    }
    return response;
    
}

char *parse_rm(__uint8_t *admin_received_msg, settings_t settings, metrics_t metrics) {
    char *response = "TODO RM";
    switch(admin_received_msg[0]) {
        case MTYPES:
            break;
    }
    return response;
    
}