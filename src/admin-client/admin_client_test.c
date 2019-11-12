#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "admin_client.h"

uint8_t *get_SET_cmd_request(char *buffer);
uint8_t *get_SET_mtypes_request(char *buffer);
uint8_t *get_GET_request(operations operation);

uint8_t *parse_login(char *buffer, size_t login_size);
uint8_t *parse_logout();
uint8_t *parse_get(char *buffer);
uint8_t *parse_set(char *buffer);
uint8_t *parse_rm(char *buffer);

uint8_t *parse_command(char *buffer);
uint8_t *parse_login_or_logout(char *buffer);

int valid_params_qty(char *mtypes_params, uint8_t mtypes_qty);

static char *TEST_SUCCESS = "Test succeeded! :) \n";
static char *TEST_FAILED = "Test failed! :( \n";

void test_valid_params_qty_with_valid_amount_of_params() {
    printf("Running test: test_valid_params_qty_with_valid_amount_of_params\n");
    char *params = "param1 param2 param3";
    uint8_t params_qty = 3;
    // printf("Calling function valid_params_qty with %s and %d\n", params, params_qty);
    int result = valid_params_qty(params, params_qty);
    if (result == 1) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_valid_params_qty_with_invalid_amount_of_params() {
    printf("Running test: test_valid_params_qty_with_valid_amount_of_params\n");
    char *params = "param1 param2";
    uint8_t params_qty = 3;
    // printf("Calling function valid_params_qty with %s and %d\n", params, params_qty);
    int result = valid_params_qty(params, params_qty);
    if (result == 0) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_get_SET_cmd_request_with_valid_buffer() {
    printf("Running test: test_get_SET_cmd_request_with_valid_buffer\n");
    char *valid_buffer = "set cmd superBigFunCommand";
    uint8_t *result = get_SET_cmd_request(valid_buffer);
    if (result != NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }

}

void test_get_SET_cmd_request_with_invalid_buffer() {
    printf("Running test: test_get_SET_cmd_request_with_invalid_buffer\n");
    char *invalid_buffer = "set cmd super Big Fun Command >";
    uint8_t *result = get_SET_cmd_request(invalid_buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_get_SET_mtypes_request_with_valid_buffer() {
    printf("Running test: test_get_SET_mtypes_request_with_valid_buffer\n");
    char *valid_buffer = "set mtypes 4 mtype1 mtype2 mtype3 mtype4";
    uint8_t *result = get_SET_mtypes_request(valid_buffer);
    if (result != NULL && result[0] == SET_REQUEST && result[1] == 
        MTYPES && result[2] == 4 && !strcmp(result+3, "mtype1,mtype2,mtype3,mtype4")) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_get_SET_mtypes_request_with_invalid_buffer() {
    printf("Running test: test_get_SET_mtypes_request_with_invalid_buffer\n");
    char *valid_buffer = "set mtypes 4 mtype1 mtype2 mtype3";
    uint8_t *result = get_SET_mtypes_request(valid_buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_get_GET_request_with_operation() {
    printf("Running test: test_get_GET_request_with_operation\n");
    uint8_t *result = get_GET_request(ACCESSES);
    int assumptions = result[0] == GET_REQUEST && result[1] == ACCESSES && result[2] == '\0' ;
    if (assumptions) { 
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_login_with_valid_buffer_and_login_size() {
    printf("Running test: test_parse_login_with_valid_buffer_and_login_size\n");
    char *valid_buffer = "login tokenazo";
    uint8_t *result = parse_login(valid_buffer, strlen(LOGIN));
    int assumptions = result != NULL && result[0] == LOGIN_REQUEST && !strcmp((char *)result+1, "tokenazo");
    if (assumptions) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_login_with_invalid_buffer_and_login_size() {
    printf("Running test: test_parse_login_with_invalid_buffer_and_login_size\n");
    char *valid_buffer = "login tokenazo peroPiola";
    uint8_t *result = parse_login(valid_buffer, strlen(LOGIN));
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_logout() {
    printf("Running test: test_parse_logout\n");
    uint8_t *result = parse_logout();
    if (result != NULL && result[0] == LOGOUT_REQUEST && result[1] == '\0') {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_get_accesses() {
    printf("Running test: test_parse_get_accesses\n");
    char *buffer = "get accesses";
    uint8_t *result = parse_get(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == ACCESSES) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}
    
void test_parse_get_bytes() {
    printf("Running test: test_parse_get_bytes\n");
    char *buffer = "get bytes";
    uint8_t *result = parse_get(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == BYTES) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_get_concurrent() {
    printf("Running test: test_parse_get_concurrent\n");
    char *buffer = "get concurrent";
    uint8_t *result = parse_get(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == CONCURRENT) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_get_mtypes() {
    printf("Running test: test_parse_get_mtypes\n");
    char *buffer = "get mtypes";
    uint8_t *result = parse_get(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == MTYPES) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_get_cmd() {
    printf("Running test: test_parse_get_cmd\n");
    char *buffer = "get cmd";
    uint8_t *result = parse_get(buffer);
    if (result != NULL && result[0] == GET_REQUEST && result[1] == CMD) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_get_with_invalid_buffer() {
    printf("Running test: test_parse_get_with_invalid_buffer\n");
    char *buffer = "get CUALQUIERcoSA";
    uint8_t *result = parse_get(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_set_with_mtypes() {
    printf("Running test: test_parse_set_with_mtypes\n");
    char *buffer = "set mtypes 2 asd asd";
    uint8_t *result = parse_set(buffer);
    if (result != NULL && result[0] == SET_REQUEST && result[1] == MTYPES) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_set_with_cmd() {
    printf("Running test: test_parse_set_with_cmd\n");
    char *buffer = "set cmd commandzo";
    uint8_t *result = parse_set(buffer);
    if (result != NULL && result[0] == SET_REQUEST && result[1] == CMD) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_set_with_invalid_buffer() {
    printf("Running test: test_parse_set_with_invalid_buffer\n");
    char *buffer = "set set 2 asd asd";
    uint8_t *result = parse_set(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_rm_mtypes() {
    printf("Running test: test_parse_rm_mtypes\n");
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

void test_parse_rm_with_invalid_buffer() {
    printf("Running test: test_parse_rm_with_invalid_buffer\n");
    char *buffer = "rm 2 asd asd";
    uint8_t *result = parse_rm(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}


void test_parse_login_or_logout_with_login() {
    printf("Running test: test_parse_login_or_logout_with_login\n");
    char *buffer = "login tokenazo";
    uint8_t *result = parse_login_or_logout(buffer);
    if (result != NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_login_or_logout_with_logout() {
    printf("Running test: test_parse_login_or_logout_with_logout\n");
    char *buffer = "logout";
    uint8_t *result = parse_login_or_logout(buffer);
    if (result != NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_login_or_logout_with_invalid_command() {
    printf("Running test: test_parse_login_or_logout_with_invalid_command\n");
    char *buffer = "login alksdja asd";
    uint8_t *result = parse_login_or_logout(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_command_with_invalid_command() {
    printf("Running test: test_parse_command_with_invalid_command\n");
    char *buffer = "login alksdja asd";
    uint8_t *result = parse_command(buffer);
    if (result == NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
}

void test_parse_command_with_valid_command() {
    printf("Running test: test_parse_command_with_valid_command\n");
    char *buffer = "login alksdja";
    uint8_t *result = parse_command(buffer);
    if (result != NULL) {
        printf("%s\n", TEST_SUCCESS);
    } else {
        printf("%s\n", TEST_FAILED);
    }
    
}

int main(void){
    test_valid_params_qty_with_valid_amount_of_params();
    printf("\n");
    test_valid_params_qty_with_invalid_amount_of_params();
    printf("\n");

    test_get_SET_cmd_request_with_valid_buffer();
    printf("\n");
    test_get_SET_cmd_request_with_invalid_buffer();
    printf("\n");

    test_get_SET_mtypes_request_with_valid_buffer();
    printf("\n");
    test_get_SET_mtypes_request_with_invalid_buffer();
    printf("\n");

    test_get_GET_request_with_operation();
    printf("\n");
    
    test_parse_login_with_valid_buffer_and_login_size();
    printf("\n");
    test_parse_login_with_invalid_buffer_and_login_size();
    printf("\n");

    test_parse_logout();
    printf("\n");

    test_parse_get_accesses();
    printf("\n");
    test_parse_get_bytes();
    printf("\n");
    test_parse_get_concurrent();
    printf("\n");
    test_parse_get_mtypes();
    printf("\n");
    test_parse_get_cmd();
    printf("\n");
    test_parse_get_with_invalid_buffer();
    printf("\n");

    test_parse_set_with_mtypes();
    printf("\n");
    test_parse_set_with_cmd();
    printf("\n");
    test_parse_set_with_invalid_buffer();
    printf("\n");

    test_parse_rm_mtypes();
    printf("\n");
    test_parse_rm_with_invalid_buffer();
    printf("\n");

    test_parse_login_or_logout_with_login();
    printf("\n");
    test_parse_login_or_logout_with_logout();
    printf("\n");
    test_parse_login_or_logout_with_invalid_command();
    printf("\n");

    test_parse_command_with_invalid_command();
    printf("\n");
    test_parse_command_with_valid_command();
    printf("\n");
}
