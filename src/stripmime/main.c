#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENV_VARIABLES_QUANTITY 2
#define FAIL -1
#define SUCCESS 1
#define TRUE 1
#define FALSE 0
#define BLOCK 10
#define MULTIPART "multipart"
#define DEFAULT_MIME_VERSION "1.0"
#define DEFAULT_MIME "text/plain"
#define HEADER_TO_BODY "\n\n"
#define NEW_LINE '\n'
#define CONTENT_TYPE "Content-Type: "
#define CONTENT_TYPE_LENGTH 14

const char * ENV_VARIABLES[] = {"FILTER_MEDIAS", "FILTER_MSG"};

int check_variables();
int check_mime(char *);

int main(void) {
    if(check_variables() == FAIL) {
        return FAIL;
    }
    char * filter_mime = getenv(ENV_VARIABLES[0]);
    int slash_position = -1;
    if(check_mime(filter_mime, &slash_position) == FAIL)
        fprintf(stderr, "El mime para filtrar está mal definido");
    char * mime_to_filter = NULL;
    
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

int check_mime(char * mime, int * slash_position) {
    int slash_flag = FALSE;
    for(int i = 0; mime[i] != '\0'; i++) {
        if(mime[i] == '/')
            if(slash_flag == TRUE)
                return FAIL;
            *slash_position = i;
            slash_flag = TRUE;
    }
    if(slash_flag = 1)
        return SUCCESS;
    return FAIL;
}

int check_pop3_headers() {
    int finished_head = FALSE;
    int head_position = 0;
    while( (c = getchar()) != EOF && !finished_head) {
        putchar(c);
        if( c == CONTENT_TYPE[head_position])
            head_position++;
        else {
            skip_to_new_line();
            head_position = 0;
        }
        if(head_position == CONTENT_TYPE_LENGTH) {

        }
}

int skip_to_new_line() {
    int c;
    int got_new_line = FALSE;
    while( (c = getchar()) != EOF && !got_new_line ) {
        putchar(c);
        if( c == '\n') 
            got_new_line == TRUE;
    }
    return got_new_line;
}
