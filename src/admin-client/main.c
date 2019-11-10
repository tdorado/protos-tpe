#include "admin_client.h"

uint8_t *parse_command(char* buffer) {
    uint8_t *command = NULL;
    switch(TOUPPER(buffer[0])) {
        case 'L': //login logout
            command = parse_login_or_logout(buffer);
            break;
        case 'G': //Get
            command = parse_get(buffer);
            break; 
        case 'S': //set
            command = parse_set(buffer);
            break;
        case 'R': //rm
            command = parse_rm(buffer);
            break;
        default:
            printf("Va a devolver null en default\n");
            return command;
    }
    printf("Va a devolver: %s %x\n", (char*)command, command[0]);
    return command;
}

uint8_t *parse_login_or_logout(char *buffer) {
    if (strncasecmp(buffer, LOGIN, strlen(LOGIN)) == 0) {
        return parse_login(buffer, strlen(LOGIN));
    } else if (strncasecmp(buffer, LOGOUT, strlen(LOGOUT)) == 0) {
        return parse_logout();
    }
    return NULL;
}

uint8_t *parse_login(char *buffer, size_t login_size) {
    if (strlen(buffer) - login_size - 1 <= 0 
        || !valid_params_qty(buffer+login_size+1, 1)){
            return NULL;
    } 
    size_t token_size = (strlen(buffer) - login_size)* sizeof(char);
    uint8_t *result = (uint8_t *) malloc(token_size + 2);
    result[0] = LOGIN_REQUEST;
    memcpy(result+1, buffer + login_size + 1, token_size+1); //token_size+1 because i want to copy '\0'
    return result;
}

uint8_t *parse_logout() {
    uint8_t *result = (uint8_t *) malloc(2);
    result[0] = LOGOUT_REQUEST;
    result[1] = '\0';
    return result;
}

uint8_t *parse_get(char *buffer) {
    if (strncasecmp(buffer, GET_ACCESSES, strlen(GET_ACCESSES)) == 0) {
        return get_GET_request(ACCESSES);
    } else if (strncasecmp(buffer, GET_BYTES, strlen(GET_BYTES)) == 0) {
        return get_GET_request(BYTES);
    } else if (strncasecmp(buffer, GET_CMD, strlen(GET_CMD)) == 0) {
        return get_GET_request(CMD);
    } else if (strncasecmp(buffer, GET_CONCURRENT, strlen(GET_CONCURRENT)) == 0) {
        return get_GET_request(CONCURRENT);
    } else if (strncasecmp(buffer, GET_MTYPES, strlen(GET_MTYPES)) == 0) {
        return get_GET_request(MTYPES);
    }
    return NULL;
}

uint8_t *get_GET_request(operations operation) {
    uint8_t *result = (uint8_t *) malloc(3);
    result[0] = GET_REQUEST;
    result[1] = operation;
    result[2] = '\0';
    return result;
}

uint8_t *parse_set(char *buffer) {
    if (strncasecmp(buffer, SET_CMD, strlen(SET_CMD)) == 0) {
        return get_SET_cmd_request(buffer);
    } else if (strncasecmp(buffer, SET_MTYPES, strlen(SET_MTYPES)) == 0) {
        return get_SET_mtypes_request(buffer);
    }
    return NULL;    
}

uint8_t *get_SET_cmd_request(char *buffer) {
    if (strlen(buffer) - strlen(SET_CMD) - 1 <= 0 
          || !valid_params_qty(buffer+strlen(SET_CMD)+1, 1)) { 
        return NULL;
    }
    size_t command_size = strlen(buffer) - strlen(SET_CMD) + 1;
    uint8_t *result = (uint8_t *) malloc(command_size + 2);
    result[0] = SET_REQUEST;
    result[1] = CMD;
    memcpy(result + 2, buffer + strlen(SET_CMD), command_size);
    return result;
}

int valid_params_qty(char *mtypes_params, uint8_t mtypes_qty) {
    int count = 0;
    int is_blank_before = 0;
    for (size_t j = 0 ; j < strlen(mtypes_params) ; j++) {
        if (!is_blank_before && (isblank(mtypes_params[j]) || j == strlen(mtypes_params) - 1)) {
            count++;
            is_blank_before = 1;
        } else {
            is_blank_before = 0;
        }
    }
    return count == mtypes_qty;
}

uint8_t *get_SET_mtypes_request(char *buffer) {
    if (!(strlen(buffer) > strlen(SET_MTYPES) + 2)) return NULL;
    uint8_t mtypes_qty = buffer[strlen(SET_MTYPES)+1] - '0';
    char *mtypes_params = buffer + strlen(SET_MTYPES)+3;
    if (!valid_params_qty(mtypes_params, mtypes_qty)) return NULL;;
    uint8_t *result = malloc(strlen(buffer) - strlen(SET_MTYPES) + 4);
    result[0] = SET_REQUEST;
    result[1] = MTYPES;
    result[2] = mtypes_qty;
    memcpy(result + 3, mtypes_params, strlen(buffer)-strlen(SET_MTYPES)-2);
    replace_spaces_with_commas(result+3);
    return result;
}

uint8_t *parse_rm(char *buffer){
    if (!(strlen(buffer) > strlen(RM_MTYPES) + 2)) return NULL;
    uint8_t mtypes_qty = buffer[strlen(RM_MTYPES)+1] - '0';
    char *mtypes_params = buffer + strlen(RM_MTYPES)+3;
    if (!valid_params_qty(mtypes_params, mtypes_qty)) return NULL;;
    uint8_t *result = malloc(strlen(buffer) - strlen(SET_MTYPES) + 4);
    result[0] = RM_REQUEST;
    result[1] = MTYPES;
    result[2] = mtypes_qty;
    memcpy(result + 3, mtypes_params, strlen(buffer)-strlen(SET_MTYPES)-2);
    replace_spaces_with_commas(result+3);
    return result;
}

void replace_spaces_with_commas(uint8_t *buffer) {
    for (size_t i = 0 ; i <= strlen((char *)buffer) ; i++) {
        if (buffer[i] == ' ') {
            buffer[i] = ',';
        }
    }
}

int main(int argc, char *argv[]) {
	int admin_socket;

	int send_status, receive_length, flags;
	struct sockaddr_in server_address;
	struct sctp_sndrcvinfo sndrcv_info;
	char buffer[BUFFER_MAX];
	int datalen = 0;
    int proxy_port = 9090;

	admin_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

	if (admin_socket == -1)
	{
		printf("Error while creating socket\n");
		exit(1);
	}

	bzero((void *)&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(proxy_port);
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	send_status = connect(admin_socket, (struct sockaddr *)&server_address, sizeof(server_address));
	while (1)
	{
		fgets(buffer, BUFFER_MAX, stdin); 
		buffer[strcspn(buffer, "\r\n")] = 0;
        uint8_t *request = parse_command(buffer);
		if (request != NULL)
		{
            datalen = strlen((char*)request) + 1;
            printf("A punto de mandar por sctp\n");
			send_status = sctp_sendmsg(admin_socket, (void *)request, (size_t)datalen, NULL, 0, 0, 0, 0, 0, 0);
			if (send_status == -1)
			{
				printf("Error in sctp_sendmsg\n");
				perror("sctp_sendmsg()");
			}
			else
				printf("Successfully sent %d bytes data to server\n", send_status);
			receive_length = sctp_recvmsg(admin_socket, buffer, sizeof(buffer), (struct sockaddr *)NULL, 0, &sndrcv_info, &flags);

			if (receive_length == -1)
			{
				printf("Error receive_length sctp_recvmsg\n");
				close(admin_socket);
				return 0;
			}
			else
			{
				buffer[receive_length] = '\0';

				printf(" Length of Data received: %d\n", receive_length);
				printf(" Data : %s\n", (char *)buffer);
			}
            free(request);
		}
		else
			printf("Invalid request!\n");
	}

	return 0;
}