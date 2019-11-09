#include "include/admin_socket.h"
#include <stdlib.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/sctp.h>
#include <sys/select.h>

#include "include/admin_socket.h"

#define BUFFER_MAX 1000
#define MAX_PENDING_CONNECTIONS 5
#define ADMIN_BUFFER_SIZE 1024 * 8
#define ENABLE 1
#define DISABLE 0

int init_admin_socket(struct sockaddr_in *server_addr, socklen_t *server_addr_len, settings_t settings)
{
  int adminSocket, ret;
  struct sockaddr_in servaddr;
  struct sctp_initmsg initmsg;

  adminSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP);
  if (adminSocket == -1)
  {
    printf("Failed to create socket\n");
    perror("socket()");
    exit(1);
  }

  bzero((void *)&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(9090);

  ret = bind(adminSocket, (struct sockaddr *)&servaddr, sizeof(servaddr));

  if (ret == -1)
  {
    printf("Bind failed \n");
    perror("bind()");
    close(adminSocket);
    exit(1);
  }

  /* Specify that a maximum of 5 streams will be available per socket */
  memset(&initmsg, 0, sizeof(initmsg));
  initmsg.sinit_num_ostreams = 5;
  initmsg.sinit_max_instreams = 5;
  initmsg.sinit_max_attempts = 4;
  ret = setsockopt(adminSocket, IPPROTO_SCTP, SCTP_INITMSG,
                   &initmsg, sizeof(initmsg));

  if (ret == -1)
  {
    printf("setsockopt() failed \n");
    perror("setsockopt()");
    close(adminSocket);
    exit(1);
  }

  ret = listen(adminSocket, 5);
  if (ret == -1)
  {
    printf("listen() failed \n");
    perror("listen()");
    close(adminSocket);
    exit(1);
  }

  return adminSocket;
}

void resolve_admin_client(int adminSocket, fd_set *readFDs, struct sockaddr_in *adminAddress, socklen_t *adminAddressLength, settings_t *st, metrics_t *mtx)
{
  if (FD_ISSET(adminSocket, readFDs))
  {
    resolveAdminClientIntern(adminSocket, adminAddress, adminAddressLength, st, mtx);
  }
}

void set_admin_fd(const int admin_fd, int *max_fd, fd_set *read_fds){
    FD_SET(admin_fd, read_fds);
    if (admin_fd > *max_fd) {
        *max_fd = admin_fd;
    }
}


void resolveAdminClientIntern(int adminSocket, struct sockaddr_in *adminAddress, int adminAddressLength, settings_t *st, metrics_t *mtx)
{
  int flags = 0;
  char adminReceivedMessage[ADMIN_BUFFER_SIZE];
  char *proxyResponseMessage;

  struct sctp_sndrcvinfo sndrcvinfo;
  int ret;

  int messageReceivedLength;

  bool auth = false;

  int connSock = accept(adminSocket, (struct sockaddr *)NULL, (unsigned int *)NULL);
  printf("Acepto conxion? %d\n", connSock);
  messageReceivedLength = sctp_recvmsg(connSock, adminReceivedMessage, ADMIN_BUFFER_SIZE, (struct sockaddr *)NULL, 0, &sndrcvinfo, &flags);
  if (messageReceivedLength == -1)
  {
    printf("Error in sctp_recvmsg\n");
    perror("sctp_recvmsg()");
  }
  else
  {
    //Add '\0' in case of text data
    adminReceivedMessage[messageReceivedLength] = '\0';
    // proxyResponseMessage = parseRequest(adminReceivedMessage, st, mtx); // TODO
    proxyResponseMessage = "HARDCODEADISIMO";
  }

  if (messageReceivedLength != 0)
  {
    /* ECHO for testing */
    ret = sctp_sendmsg(connSock, (void *)proxyResponseMessage, (size_t)BUFFER_MAX,
                       NULL, 0, 0, 0, 0, 0, 0);
    if (ret == -1)
    {
      printf("Error in sctp_sendmsg\n");
      perror("sctp_sendmsg()");
    }
    else
      printf("Successfully sent %d bytes data to admin\n", ret);
    // free(proxyResponseMessage);
  }
}
