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

#define TOUPPER(c) c - 32
#define BUFFER_MAX 2048

typedef enum requestType {
    LOGIN_REQUEST = 0x01,
    LOGOUT_REQUEST, 
    GET_REQUEST, 
    SET_REQUEST,
    RM_REQUEST
} requestType;

typedef enum operations {
    CONCURRENT = 0X01,
    ACCESSES,
    BYTES,
    MTYPES,
    CMD
} operations;

typedef enum response_type { // is equivalent to request type, it kinda be duplicated code, but we win on expresiveness.
    LOGIN_RESPONSE = 0x01,
    LOGOUT_RESPONSE,
    GET_RESPONSE,
    SET_RESPONSE,
    RM_RESPONSE
} response_type;

// LOGIN LOGOUT
static char *LOGIN = "LOGIN";  // LOGIN <token>
static char *LOGOUT = "LOGOUT";
// GETTERS
static char *GET_CONCURRENT = "GET CONCURRENT";
static char *GET_ACCESSES = "GET ACCESSES";
static char *GET_BYTES = "GET BYTES";
static char *GET_MTYPES = "GET MTYPES";
static char *GET_CMD = "GET CMD";
// SETTERS
static char *SET_MTYPES = "SET MTYPES"; //set mtypes paramsQty [params]
static char *SET_CMD = "SET CMD";  // SET CMD <CMD>
// RM
static char *RM_MTYPES = "RM MTYPES"; // rm mtypes paramsQty [params]

__uint8_t *parseCommand(char *buffer);
__uint8_t *parseLoginOrLogout(char *buffer);
__uint8_t *parseLogin(char *buffer, size_t login_size);
__uint8_t *parseLogout(void);
__uint8_t *parseGet(char *buffer);
__uint8_t *parseSet(char *buffer);
__uint8_t *parseRm(char *buffer);
__uint8_t *getGETRequest(operations operation);
__uint8_t *getSETMtypesRequest(char *buffer);
__uint8_t *getSETCmdRequest(char *buffer);
int validParamsQty(char *mtypesParams, __uint8_t mtypesQty);
void replaceSpacesWithCommas(__uint8_t *buffer);

int strncasecmp(const char *s1, const char *s2, size_t n);
void bzero(void *s, size_t n);