#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "adminClient.h"

// In order to run test, you should comment main function in adminClient.c 

__uint8_t *getSETCmdRequest(char *buffer);
__uint8_t *getSETMtypesRequest(char *buffer);
__uint8_t *getGETRequest(operations operation);

__uint8_t *parseLogin(char *buffer, size_t login_size);
__uint8_t *parseLogout();
__uint8_t *parseGet(char *buffer);
__uint8_t *parseSet(char *buffer);
__uint8_t *parseRm(char *buffer);

__uint8_t *parseCommand(char *buffer);
__uint8_t *parseLoginOrLogout(char *buffer);

int validParamsQty(char *mtypesParams, __uint8_t mtypesQty);

static char *TEST_SUCCESS = "Test succeeded! :) \n";
static char *TEST_FAILED = "Test failed! :( \n";

void testValidParamsQtyWithValidAmountOfParams() {
    printf("Running test: testValidParamsQtyWithValidAmountOfParams\n");
    char *params = "param1 param2 param3";
    __uint8_t paramsQty = 3;
    // printf("Calling function validParamsQty with %s and %d\n", params, paramsQty);
    int result = validParamsQty(params, paramsQty);
    if (result == 1) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testValidParamsQtyWithInvalidAmountOfParams() {
    printf("Running test: testValidParamsQtyWithValidAmountOfParams\n");
    char *params = "param1 param2";
    __uint8_t paramsQty = 3;
    // printf("Calling function validParamsQty with %s and %d\n", params, paramsQty);
    int result = validParamsQty(params, paramsQty);
    if (result == 0) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testGetSETCmdRequestWithValidBuffer() {
    printf("Running test: testGetSETCmdRequestWithValidBuffer\n");
    char *validBuffer = "set cmd superBigFunCommand";
    __uint8_t *result = getSETCmdRequest(validBuffer);
    if (result != NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }

}

void testGetSETCmdRequestWithInvalidBuffer() {
    printf("Running test: testGetSETCmdRequestWithInvalidBuffer\n");
    char *invalidBuffer = "set cmd super Big Fun Command >";
    __uint8_t *result = getSETCmdRequest(invalidBuffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testGetSETMtypesRequestWithValidBuffer() {
    printf("Running test: testGetSETMtypesRequestWithValidBuffer\n");
    char *validBuffer = "set mtypes 4 mtype1 mtype2 mtype3 mtype4";
    __uint8_t *result = getSETMtypesRequest(validBuffer);
    if (result != NULL && result[0] == SET_REQUEST && result[1] == 
        MTYPES && result[2] == 4 && !strcmp(result+3, "mtype1,mtype2,mtype3,mtype4")) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testGetSETMtypesRequestWithInvalidBuffer() {
    printf("Running test: testGetSETMtypesRequestWithInvalidBuffer\n");
    char *validBuffer = "set mtypes 4 mtype1 mtype2 mtype3";
    __uint8_t *result = getSETMtypesRequest(validBuffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testGetGETRequestWithOperation() {
    printf("Running test: testGetGETRequestWithOperation\n");
    __uint8_t *result = getGETRequest(ACCESSES);
    int assumptions = result[0] == GET_REQUEST && result[1] == ACCESSES && result[2] == '\0' ;
    if (assumptions) { 
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseLoginWithValidBufferAndLoginSize() {
    printf("Running test: testParseLoginWithValidBufferAndLoginSize\n");
    char *validBuffer = "login tokenazo";
    __uint8_t *result = parseLogin(validBuffer, strlen(LOGIN));
    int assumptions = result != NULL && result[0] == LOGIN_REQUEST && !strcmp((char *)result+1, "tokenazo");
    if (assumptions) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseLoginWithInvalidBufferAndLoginSize() {
    printf("Running test: testParseLoginWithInvalidBufferAndLoginSize\n");
    char *validBuffer = "login tokenazo peroPiola";
    __uint8_t *result = parseLogin(validBuffer, strlen(LOGIN));
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseLogout() {
    printf("Running test: testParseLogout\n");
    __uint8_t *result = parseLogout();
    if (result != NULL && result[0] == LOGOUT_REQUEST && result[1] == '\0') {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseGetAccesses() {
    printf("Running test: testParseGetAccesses\n");
    char *buffer = "get accesses";
    __uint8_t *result = parseGet(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == ACCESSES) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}
    
void testParseGetBytes() {
    printf("Running test: testParseGetBytes\n");
    char *buffer = "get bytes";
    __uint8_t *result = parseGet(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == BYTES) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseGetConcurrent() {
    printf("Running test: testParseGetConcurrent\n");
    char *buffer = "get concurrent";
    __uint8_t *result = parseGet(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == CONCURRENT) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseGetMtypes() {
    printf("Running test: testParseGetMtypes\n");
    char *buffer = "get mtypes";
    __uint8_t *result = parseGet(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == MTYPES) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseGetCmd() {
    printf("Running test: testParseGetCmd\n");
    char *buffer = "get cmd";
    __uint8_t *result = parseGet(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == CMD) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseGetWithInvalidBuffer() {
    printf("Running test: testParseGetAccesses\n");
    char *buffer = "get CUALQUIERcoSA";
    __uint8_t *result = parseGet(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseSetWithMtypes() {
    printf("Running test: testParseSetWithMtypes\n");
    char *buffer = "set mtypes 2 asd asd";
    __uint8_t *result = parseSet(buffer);
    if (result != NULL && result[0] == SET_REQUEST && result[1] == MTYPES) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseSetWithCmd() {
    printf("Running test: testParseSetWithCmd\n");
    char *buffer = "set cmd commandzo";
    __uint8_t *result = parseSet(buffer);
    if (result != NULL && result[0] == SET_REQUEST && result[1] == CMD) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseSetWithInvalidBuffer() {
    printf("Running test: testParseSetWithInvalidBuffer\n");
    char *buffer = "set set 2 asd asd";
    __uint8_t *result = parseSet(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseRmMtypes() {
    printf("Running test: testParseRmMtypes\n");
    char *buffer = "rm mtypes 3 este elOtro esteOtro";
    __uint8_t *result = parseRm(buffer);
    int firstAssumption = result != NULL && result[0] == RM_REQUEST && result[1] == MTYPES;
    int secondAssumption = result[2] == 3 && !strcmp(result+3, "este,elOtro,esteOtro");
    if (firstAssumption && secondAssumption) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseRmWithInvalidBuffer() {
    printf("Running test: testParseRmWithInvalidBuffer\n");
    char *buffer = "rm 2 asd asd";
    __uint8_t *result = parseRm(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}


void testParseLoginOrLogoutWithLogin() {
    printf("Running test: testParseLoginOrLogoutWithLogin\n");
    char *buffer = "login tokenazo";
    __uint8_t *result = parseLoginOrLogout(buffer);
    if (result != NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseLoginOrLogoutWithLogout() {
    printf("Running test: testParseLoginOrLogoutWithLogout\n");
    char *buffer = "logout";
    __uint8_t *result = parseLoginOrLogout(buffer);
    if (result != NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseLoginOrLogoutWithInvalidCommand() {
    printf("Running test: testParseLoginOrLogoutWithInvalidCommand\n");
    char *buffer = "login alksdja asd";
    __uint8_t *result = parseLoginOrLogout(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseCommandWithInvalidCommand() {
    printf("Running test: testParseCommandWithInvalidCommand\n");
    char *buffer = "login alksdja asd";
    __uint8_t *result = parseCommand(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void testParseCommandWithValidCommand() {
    printf("Running test: testParseCommandWithValidCommand\n");
    char *buffer = "login alksdja";
    __uint8_t *result = parseCommand(buffer);
    if (result != NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
    
}

int main(int argc, char *argv[]){
    testValidParamsQtyWithValidAmountOfParams();
    printf("\n");
    testValidParamsQtyWithInvalidAmountOfParams();
    printf("\n");

    testGetSETCmdRequestWithValidBuffer();
    printf("\n");
    testGetSETCmdRequestWithInvalidBuffer();
    printf("\n");

    testGetSETMtypesRequestWithValidBuffer();
    printf("\n");
    testGetSETMtypesRequestWithInvalidBuffer();
    printf("\n");

    testGetGETRequestWithOperation();
    printf("\n");
    
    testParseLoginWithValidBufferAndLoginSize();
    printf("\n");
    testParseLoginWithInvalidBufferAndLoginSize();
    printf("\n");

    testParseLogout();
    printf("\n");

    testParseGetAccesses();
    printf("\n");
    testParseGetBytes();
    printf("\n");
    testParseGetConcurrent();
    printf("\n");
    testParseGetMtypes();
    printf("\n");
    testParseGetCmd();
    printf("\n");
    testParseGetWithInvalidBuffer();
    printf("\n");

    testParseSetWithMtypes();
    printf("\n");
    testParseSetWithCmd();
    printf("\n");
    testParseSetWithInvalidBuffer();
    printf("\n");

    testParseRmMtypes();
    printf("\n");
    testParseRmWithInvalidBuffer();
    printf("\n");

    testParseLoginOrLogoutWithLogin();
    printf("\n");
    testParseLoginOrLogoutWithLogout();
    printf("\n");
    testParseLoginOrLogoutWithInvalidCommand();
    printf("\n");

    testParseCommandWithInvalidCommand();
    printf("\n");
    testParseCommandWithValidCommand();
    printf("\n");
}
