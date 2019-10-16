#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "adminClient.h"

#define TOUPPER(c) c - 32

__uint8_t *parseCommand(char *buffer);
__uint8_t *parseLoginOrLogout(char *buffer);
__uint8_t *parseLogin(char *buffer, size_t login_size);
__uint8_t *parseLogout();
__uint8_t *parseGet(char *buffer);
__uint8_t *parseSet(char *buffer);
__uint8_t *parseRm(char *buffer);
__uint8_t *getGETRequest(operations operation);
__uint8_t *getSETMtypesRequest(char *buffer);
__uint8_t *getSETCmdRequest(char *buffer);
void printCharMatrix(char **matrix, int params_qty);
char **getMatrixOfParams(char* response, int params_qty);
int validParamsQty(char *mtypesParams, __uint8_t mtypesQty);
void replaceSpacesWithCommas(char *buffer);

__uint8_t *parseCommand(char* buffer) {
    __uint8_t *command = NULL;

    switch(TOUPPER(buffer[0])) {
        case 'L': //login logout
            command = parseLoginOrLogout(buffer);
            break;
        case 'G': //Get
            command = parseGet(buffer);
            break; 
        case 'S': //set
            command = parseSet(buffer);
            break;
        case 'R': //rm
            command = parseRm(buffer);
            break;
    }

    return command;
}

__uint8_t *parseLoginOrLogout(char *buffer) {
    if (strncasecmp(buffer, LOGIN, strlen(LOGIN)) == 0) {
        return parseLogin(buffer, strlen(LOGIN));
    } else if (strncasecmp(buffer, LOGOUT, strlen(LOGOUT)) == 0) {
        return parseLogout();
    }
    return NULL;
}

__uint8_t *parseLogin(char *buffer, size_t login_size) {
    if (strlen(buffer) - login_size - 1 <= 0 
        || !validParamsQty(buffer+login_size+1, 1)){
            return NULL;
    } 
    size_t token_size = (strlen(buffer) - login_size)* sizeof(char);
    __uint8_t *result = (__uint8_t *) malloc(token_size + 2);
    result[0] = LOGIN_REQUEST;
    memcpy(result+1, buffer + login_size + 1, token_size+1); //token_size+1 because i want to copy '\0'
    return result;
}

__uint8_t *parseLogout() {
    __uint8_t *result = (__uint8_t *) malloc(2);
    result[0] = LOGOUT_REQUEST;
    result[1] = '\0';
    return result;
}

__uint8_t *parseGet(char *buffer) {
    if (strncasecmp(buffer, GET_ACCESSES, strlen(GET_ACCESSES)) == 0) {
        return getGETRequest(ACCESSES);
    } else if (strncasecmp(buffer, GET_BYTES, strlen(GET_BYTES)) == 0) {
        return getGETRequest(BYTES);
    } else if (strncasecmp(buffer, GET_CMD, strlen(GET_CMD)) == 0) {
        return getGETRequest(CMD);
    } else if (strncasecmp(buffer, GET_CONCURRENT, strlen(GET_CONCURRENT)) == 0) {
        return getGETRequest(CONCURRENT);
    } else if (strncasecmp(buffer, GET_MTYPES, strlen(GET_MTYPES)) == 0) {
        return getGETRequest(MTYPES);
    }
    return NULL;
}

__uint8_t *getGETRequest(operations operation) {
    __uint8_t *result = (__uint8_t *) malloc(3);
    result[0] = GET_REQUEST;
    result[1] = operation;
    result[2] = '\0';
    return result;
}

__uint8_t *parseSet(char *buffer) {
    if (strncasecmp(buffer, SET_CMD, strlen(SET_CMD)) == 0) {
        return getSETCmdRequest(buffer);
    } else if (strncasecmp(buffer, SET_MTYPES, strlen(SET_MTYPES)) == 0) {
        return getSETMtypesRequest(buffer);
    }
    return NULL;    
}

__uint8_t *getSETCmdRequest(char *buffer) {
    if (strlen(buffer) - strlen(SET_CMD) - 1 <= 0 
          || !validParamsQty(buffer+strlen(SET_CMD)+1, 1)) { 
        return NULL;
    }
    size_t command_size = strlen(buffer) - strlen(SET_CMD) + 1;
    char *command = malloc(command_size);
    strncpy(command, buffer + strlen(SET_CMD), command_size);
    __uint8_t *result = (__uint8_t *) malloc(command_size + 2);
    result[0] = SET_REQUEST;
    result[1] = CMD;
    memcpy(result + 2, command, command_size);
    return result;
}

int validParamsQty(char *mtypesParams, __uint8_t mtypesQty) {
    int count = 0;
    int isBlankBefore = 0;
    for (int j = 0 ; j < strlen(mtypesParams) ; j++) {
        if (!isBlankBefore && (isblank(mtypesParams[j]) || j == strlen(mtypesParams) - 1)) {
            count++;
            isBlankBefore = 1;
        } else {
            isBlankBefore = 0;
        }
    }
    return count == mtypesQty;
}

__uint8_t *getSETMtypesRequest(char *buffer) {
    if (!strlen(buffer) > strlen(SET_MTYPES) + 2) return NULL;
    __uint8_t mtypes_qty = buffer[strlen(SET_MTYPES)+1] - '0';
    char *mtypesParams = buffer + strlen(SET_MTYPES)+3;
    if (!validParamsQty(mtypesParams, mtypes_qty)) return NULL;;
    __uint8_t *result = malloc(strlen(buffer) - strlen(SET_MTYPES) + 4);
    result[0] = SET_REQUEST;
    result[1] = MTYPES;
    result[2] = mtypes_qty;
    memcpy(result + 3, mtypesParams, strlen(buffer)-strlen(SET_MTYPES)-2);
    replaceSpacesWithCommas(result+3);
    return result;
}

__uint8_t *parseRm(char *buffer){
    if (!strlen(buffer) > strlen(RM_MTYPES) + 2) return NULL;
    __uint8_t mtypes_qty = buffer[strlen(RM_MTYPES)+1] - '0';
    char *mtypesParams = buffer + strlen(RM_MTYPES)+3;
    if (!validParamsQty(mtypesParams, mtypes_qty)) return NULL;;
    __uint8_t *result = malloc(strlen(buffer) - strlen(SET_MTYPES) + 4);
    result[0] = RM_REQUEST;
    result[1] = MTYPES;
    result[2] = mtypes_qty;
    memcpy(result + 3, mtypesParams, strlen(buffer)-strlen(SET_MTYPES)-2);
    replaceSpacesWithCommas(result+3);
    return result;
}

void replaceSpacesWithCommas(char *buffer) {
    for (int i = 0 ; i <= strlen(buffer) ; i++) {
        if (buffer[i] == ' ') {
            buffer[i] = ',';
        }
    }
}

char **getMatrixOfParams(char* response, int params_qty) {
    int i = 0;
    char *current_param = response;
    char **params = malloc(params_qty * sizeof(char*));
    while (i < params_qty) {
        params[i] = malloc(strlen(current_param) + 1);
        strncpy(params[i], current_param, strlen(current_param));
        current_param = current_param + strlen(current_param) + 1;
        i++;
    }
    return params;
}

void printCharMatrix(char **matrix, int params_qty) {
    printf("Mtypes parameters: ");
    for (int i = 0 ; i < params_qty ; i++) {
        printf("%s ", matrix[i]);
        free(matrix[i]);
    }
    printf("\n");
}

void printResponse(char *response) {
    switch (response[0]) {
        case CONCURRENT:
            printf("%s", response+1);
            return;
        case ACCESSES:
            printf("%s", response+1);
            return;
        case BYTES:
            printf("%s", response+1);
            return;
        case CMD:
            printf("%s", response+1);
            return;
        case MTYPES:
            printCharMatrix(getMatrixOfParams(response+2, response[1]-'0'), response[1]-'0');
            return;
        default:
            printf("THERE WAS AN ERROR");
            return;
    }
}

int main(int argc, char *argv[])
{
	int admin_socket;

	int listen_sock, ret, in, flags, i;
	struct sockaddr_in server_address;
	struct sctp_status status;
	struct sctp_sndrcvinfo sndrcv_info;
	char buffer[50000];
	int datalen = 0;
    int proxy_port;

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

	// ret = connect(admin_socket, (struct sockaddr *)&server_address, sizeof(server_address));

	char *r;

	while (1)
	{
		fgets(buffer, 50000, stdin); // arreglá esto, sorete
		buffer[strcspn(buffer, "\r\n")] = 0;
        __uint8_t *request = parseCommand(buffer);
        printf("request:%s\n", request);
		if (request != NULL)
		{
			// ret = sctp_sendmsg(admin_socket, (void *)request, (size_t)datalen, NULL, 0, 0, 0, 0, 0, 0);
			// if (ret == -1)
			// {
			// 	printf("Error in sctp_sendmsg\n");
			// 	perror("sctp_sendmsg()");
			// }
			// else
			// 	printf("Successfully sent %d bytes data to server\n", ret);
			// in = sctp_recvmsg(admin_socket, buffer, sizeof(buffer), (struct sockaddr *)NULL, 0, &sndrcv_info, &flags);

			// if (in == -1)
			// {
			// 	printf("Error in sctp_recvmsg\n");
			// 	close(admin_socket);
			// 	return 0;
			// }
			// else
			// {
				// buffer[in] = '\0';

				// printf(" Length of Data received: %d\n", in);
				// printf(" Data : %s\n", (char *)buffer);
                // printResponse(buffer);
			// }
		}
		else
			printf("Invalid request!\n");
	}

	free(r);

	return 0;
}