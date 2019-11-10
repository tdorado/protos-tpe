#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENV_VARIABLES_QUANTITY 2
#define FAIL -1
#define SUCCESS 1

const char * ENV_VARIABLES[] = {"FILTER_MEDIAS", "FILTER_MSG"};

int check_variables();
int check_mime(char *);

int main(void) {
    if(check_variables() == FAIL) {
        return FAIL;
    }
    char * filter_mime = getenv(ENV_VARIABLES[0]);
    if(check_mime(filter_mime) == FAIL)
        fprintf(stderr, "El mime para filtrar está mal definido");
}

int check_variables() {
    for(int i = 0; i < ENV_VARIABLES_QUANTITY; i++) {
        if(getenv(ENV_VARIABLES[i]) == NULL) {
            fprintf(stderr, "La variable %s no está definida \n", ENV_VARIABLES[i]);
            return FAIL;
        }
    }
    return SUCCESS;
}

int check_mime(char * mime) {
    int slash_flag = 0;
    for(int i = 0; mime[i] != '\0'; i++) {
        if(mime[i] = '/')
            if(slash_flag == 1)
                return FAIL;
            slash_flag = 1;
    }
    if(slash_flag = 1)
        return SUCCESS;
    return FAIL;
}