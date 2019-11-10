#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENV_VARIABLES_QUANTITY 2
#define FAIL -1
#define SUCCESS 1

const char * ENV_VARIABLES[] = {"FILTER_MEDIAS", "FILTER_MSG"};

int check_variables();

int main(void) {
    if(check_variables() == FAIL) {
        return FAIL;
    }
}

int check_variables() {
    for(int i = 0; i < ENV_VARIABLES_QUANTITY; i++) {
        if(getenv(ENV_VARIABLES[0]) == NULL) {
            fprintf(stderr, "La variable %s no está definida \n", ENV_VARIABLES[i]);
            return FAIL;
        }
    }
    return SUCCESS;
}