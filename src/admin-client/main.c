#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/sctp.h>
#include <fcntl.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

#include "include/admin_client_parser.h"

int main(int argc, char *argv[]) {

    struct addr_and_port addr_port;
    
    if(validate_and_set_params(argc, argv, &addr_port)){
        exit(EXIT_FAILURE);
    }

	int connection_fd = -1;
	int received_len = -1;
	struct sockaddr_in server_address;
	char msg_received[BUFFER_MAX];
    char msg_to_send[BUFFER_MAX];
	int to_send_len = 0;
    int send_len = -1;
    bool stop = false;
    state_t state;

	connection_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);

	if (connection_fd == -1) {
		printf("Error while creating socket\n");
		exit(EXIT_FAILURE);
	}

	memset((void*)&server_address, 0, sizeof(struct sockaddr_in));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(addr_port.port);
	server_address.sin_addr.s_addr = inet_addr(addr_port.addr);
	connect(connection_fd, (struct sockaddr *)&server_address, sizeof(server_address));

    received_len = sctp_recvmsg(connection_fd, msg_received, BUFFER_MAX, (struct sockaddr *)NULL, 0, 0, 0);

	if (received_len == -1) {
		printf("Error receiving greeting\n");
        perror("Error recieving greeting");
		close(connection_fd);
		exit(EXIT_FAILURE);
	}
	else {
		stop = parse_greeting(msg_received, received_len);
	}

	while (!stop) {
		fgets(msg_received, BUFFER_MAX, stdin); 
		msg_received[strcspn(msg_received, "\r\n")] = 0;
        state = parse_command(msg_received, msg_to_send, &to_send_len);

		if (state != INVALID && state != HELP) {
			send_len = sctp_sendmsg(connection_fd, msg_to_send, to_send_len, NULL, 0, 0, 0, 0, 0, 0);
			if (send_len == -1) {
				printf("Error sending message\n");
				perror("Error sending message");
			}
                
			received_len = sctp_recvmsg(connection_fd, msg_received, BUFFER_MAX, (struct sockaddr *)NULL, 0, 0, 0);

			if (received_len == -1) {
				printf("Error receiving message...  closing\n");
                perror("Error receiving message");
				close(connection_fd);
				exit(EXIT_FAILURE);
			}
			else {
                stop = interpret_response(state, msg_received);
			}
		}
		else if(state == INVALID) {
			printf("Invalid request, write help for options.\n");
        }
	}
    
    close(connection_fd);

	return 0;
}