#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char * ENV_VARIABLES[] = {"FILTER_MEDIAS", "FILTER_MSG"};
    printf("FILTER_MEDIAS: %s \n", getenv(ENV_VARIABLES[0]));
    printf("FILTER_MSG: %s \n", getenv(ENV_VARIABLES[1]));
    char * filter_medias = getenv(ENV_VARIABLES[0]);
    if(filter_medias == NULL) {
        printf("Hola");
    }
}