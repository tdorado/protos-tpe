#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "admin_client.h"

// In order to run test, you should comment main function in adminClient.c 

uint8_t *get_SET_cmd_request(char *buffer);
uint8_t *get_SET_mtypes_request(char *buffer);
uint8_t *get_GET_request(operations operation);

uint8_t *parse_login(char *buffer, size_t login_size);
uint8_t *parse_logout();
uint8_t *parse_get(char *buffer);
uint8_t *parse_set(char *buffer);
uint8_t *parse_rm(char *buffer);

uint8_t *parseCommand(char *buffer);
uint8_t *parse_login_or_logout(char *buffer);

int valid_params_qty(char *mtypesParams, uint8_t mtypesQty);

static char *TEST_SUCCESS = "Test succeeded! :) \n";
static char *TEST_FAILED = "Test failed! :( \n";

void testvalid_params_qtyWithValidAmountOfParams() {
    printf("Running test: testvalid_params_qtyWithValidAmountOfParams\n");
    char *params = "param1 param2 param3";
    uint8_t paramsQty = 3;
    // printf("Calling function valid_params_qty with %s and %d\n", params, paramsQty);
    int result = valid_params_qty(params, paramsQty);
    if (result == 1) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testvalid_params_qtyWithInvalidAmountOfParams() {
    printf("Running test: testvalid_params_qtyWithValidAmountOfParams\n");
    char *params = "param1 param2";
    uint8_t paramsQty = 3;
    // printf("Calling function valid_params_qty with %s and %d\n", params, paramsQty);
    int result = valid_params_qty(params, paramsQty);
    if (result == 0) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testget_SET_cmd_requestWithValidBuffer() {
    printf("Running test: testget_SET_cmd_requestWithValidBuffer\n");
    char *validBuffer = "set cmd superBigFunCommand";
    uint8_t *result = get_SET_cmd_request(validBuffer);
    if (result != NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }

}

void testget_SET_cmd_requestWithInvalidBuffer() {
    printf("Running test: testget_SET_cmd_requestWithInvalidBuffer\n");
    char *invalidBuffer = "set cmd super Big Fun Command >";
    uint8_t *result = get_SET_cmd_request(invalidBuffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testget_SET_mtypes_requestWithValidBuffer() {
    printf("Running test: testget_SET_mtypes_requestWithValidBuffer\n");
    char *validBuffer = "set mtypes 4 mtype1 mtype2 mtype3 mtype4";
    uint8_t *result = get_SET_mtypes_request(validBuffer);
    if (result != NULL && result[0] == SET_REQUEST && result[1] == 
        MTYPES && result[2] == 4 && !strcmp(result+3, "mtype1,mtype2,mtype3,mtype4")) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testget_SET_mtypes_requestWithInvalidBuffer() {
    printf("Running test: testget_SET_mtypes_requestWithInvalidBuffer\n");
    char *validBuffer = "set mtypes 4 mtype1 mtype2 mtype3";
    uint8_t *result = get_SET_mtypes_request(validBuffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testget_GET_requestWithOperation() {
    printf("Running test: testget_GET_requestWithOperation\n");
    uint8_t *result = get_GET_request(ACCESSES);
    int assumptions = result[0] == GET_REQUEST && result[1] == ACCESSES && result[2] == '\0' ;
    if (assumptions) { 
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_loginWithValidBufferAndLoginSize() {
    printf("Running test: testparse_loginWithValidBufferAndLoginSize\n");
    char *validBuffer = "login tokenazo";
    uint8_t *result = parse_login(validBuffer, strlen(LOGIN));
    int assumptions = result != NULL && result[0] == LOGIN_REQUEST && !strcmp((char *)result+1, "tokenazo");
    if (assumptions) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_loginWithInvalidBufferAndLoginSize() {
    printf("Running test: testparse_loginWithInvalidBufferAndLoginSize\n");
    char *validBuffer = "login tokenazo peroPiola";
    uint8_t *result = parse_login(validBuffer, strlen(LOGIN));
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_logout() {
    printf("Running test: testparse_logout\n");
    uint8_t *result = parse_logout();
    if (result != NULL && result[0] == LOGOUT_REQUEST && result[1] == '\0') {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_getAccesses() {
    printf("Running test: testparse_getAccesses\n");
    char *buffer = "get accesses";
    uint8_t *result = parse_get(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == ACCESSES) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}
    
void testparse_getBytes() {
    printf("Running test: testparse_getBytes\n");
    char *buffer = "get bytes";
    uint8_t *result = parse_get(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == BYTES) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_getConcurrent() {
    printf("Running test: testparse_getConcurrent\n");
    char *buffer = "get concurrent";
    uint8_t *result = parse_get(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == CONCURRENT) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_getMtypes() {
    printf("Running test: testparse_getMtypes\n");
    char *buffer = "get mtypes";
    uint8_t *result = parse_get(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == MTYPES) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_getCmd() {
    printf("Running test: testparse_getCmd\n");
    char *buffer = "get cmd";
    uint8_t *result = parse_get(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == CMD) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_getWithInvalidBuffer() {
    printf("Running test: testparse_getAccesses\n");
    char *buffer = "get CUALQUIERcoSA";
    uint8_t *result = parse_get(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_setWithMtypes() {
    printf("Running test: testparse_setWithMtypes\n");
    char *buffer = "set mtypes 2 asd asd";
    uint8_t *result = parse_set(buffer);
    if (result != NULL && result[0] == SET_REQUEST && result[1] == MTYPES) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_setWithCmd() {
    printf("Running test: testparse_setWithCmd\n");
    char *buffer = "set cmd commandzo";
    uint8_t *result = parse_set(buffer);
    if (result != NULL && result[0] == SET_REQUEST && result[1] == CMD) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_setWithInvalidBuffer() {
    printf("Running test: testparse_setWithInvalidBuffer\n");
    char *buffer = "set set 2 asd asd";
    uint8_t *result = parse_set(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_rmMtypes() {
    printf("Running test: testparse_rmMtypes\n");
    char *buffer = "rm mtypes 3 este elOtro esteOtro";
    uint8_t *result = parse_rm(buffer);
    int firstAssumption = result != NULL && result[0] == RM_REQUEST && result[1] == MTYPES;
    int secondAssumption = result[2] == 3 && !strcmp(result+3, "este,elOtro,esteOtro");
    if (firstAssumption && secondAssumption) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_rmWithInvalidBuffer() {
    printf("Running test: testparse_rmWithInvalidBuffer\n");
    char *buffer = "rm 2 asd asd";
    uint8_t *result = parse_rm(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}


void testparse_login_or_logoutWithLogin() {
    printf("Running test: testparse_login_or_logoutWithLogin\n");
    char *buffer = "login tokenazo";
    uint8_t *result = parse_login_or_logout(buffer);
    if (result != NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_login_or_logoutWithLogout() {
    printf("Running test: testparse_login_or_logoutWithLogout\n");
    char *buffer = "logout";
    uint8_t *result = parse_login_or_logout(buffer);
    if (result != NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testparse_login_or_logoutWithInvalidCommand() {
    printf("Running test: testparse_login_or_logoutWithInvalidCommand\n");
    char *buffer = "login alksdja asd";
    uint8_t *result = parse_login_or_logout(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseCommandWithInvalidCommand() {
    printf("Running test: testParseCommandWithInvalidCommand\n");
    char *buffer = "login alksdja asd";
    uint8_t *result = parseCommand(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseCommandWithValidCommand() {
    printf("Running test: testParseCommandWithValidCommand\n");
    char *buffer = "login alksdja";
    uint8_t *result = parseCommand(buffer);
    if (result != NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
    
}

int main(int argc, char *argv[]){
    testvalid_params_qtyWithValidAmountOfParams();
    printf("\n");
    testvalid_params_qtyWithInvalidAmountOfParams();
    printf("\n");

    testget_SET_cmd_requestWithValidBuffer();
    printf("\n");
    testget_SET_cmd_requestWithInvalidBuffer();
    printf("\n");

    testget_SET_mtypes_requestWithValidBuffer();
    printf("\n");
    testget_SET_mtypes_requestWithInvalidBuffer();
    printf("\n");

    testget_GET_requestWithOperation();
    printf("\n");
    
    testparse_loginWithValidBufferAndLoginSize();
    printf("\n");
    testparse_loginWithInvalidBufferAndLoginSize();
    printf("\n");

    testparse_logout();
    printf("\n");

    testparse_getAccesses();
    printf("\n");
    testparse_getBytes();
    printf("\n");
    testparse_getConcurrent();
    printf("\n");
    testparse_getMtypes();
    printf("\n");
    testparse_getCmd();
    printf("\n");
    testparse_getWithInvalidBuffer();
    printf("\n");

    testparse_setWithMtypes();
    printf("\n");
    testparse_setWithCmd();
    printf("\n");
    testparse_setWithInvalidBuffer();
    printf("\n");

    testparse_rmMtypes();
    printf("\n");
    testparse_rmWithInvalidBuffer();
    printf("\n");

    testparse_login_or_logoutWithLogin();
    printf("\n");
    testparse_login_or_logoutWithLogout();
    printf("\n");
    testparse_login_or_logoutWithInvalidCommand();
    printf("\n");

    testParseCommandWithInvalidCommand();
    printf("\n");
    testParseCommandWithValidCommand();
    printf("\n");
}
