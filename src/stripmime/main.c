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

struct content_type {
    char * name;
    char * boundary;
};

int check_variables();
int check_mime(char *, int *);
char * check_pop3_headers();
int skip_to_new_line();
int skip_to_body();

int main(void) {
    int c;
    char * mime_type = NULL;
    if(check_variables() == FAIL) {
        return FAIL;
    }
    char * filter_mime = getenv(ENV_VARIABLES[0]);
    int slash_position = -1;
    if(check_mime(filter_mime, &slash_position) == FAIL)
        fprintf(stderr, "El mime de FILTER_MEDIAS está mal definido \n");
    char * mime_to_filter = check_pop3_headers();
    if(strncmp(mime_to_filter, "multipart/", 10) == 0) {
        printf("MULTIPARTE");
    }
    else if(strcmp(mime_to_filter, getenv(ENV_VARIABLES[0])) == 0) {
        printf(getenv(ENV_VARIABLES[1]));
        printf("\r\n.\r\n");
    }
    else  {
        while((c=getchar()) != EOF)
            putchar(c);
    }
    
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
        if(mime[i] == '/') {
            if(slash_flag == TRUE)
                return FAIL;
            *slash_position = i;
            slash_flag = TRUE;
        }
    }
    if(slash_flag = 1)
        return SUCCESS;
    return FAIL;
}

char * check_pop3_headers() {
    int chars_read = 0;
    int header_found = FALSE;
    char * mime = malloc(127);
    while(!header_found) {
        chars_read = scanf("Content-Type: %127s", mime);
        if(chars_read > 0) {
            printf("Content-Type: %s\r\n", mime);
            getchar();
            getchar();
            skip_to_body();
            return mime;
        }
        else {
            skip_to_new_line();
        }
        if(chars_read == EOF) {
            return NULL;
        }
    }
    return NULL;
}

int skip_to_new_line() {
    int c;
    while( (c = getchar()) != EOF && c != '\n' ) {
        putchar(c);
    }
    if(c == '\n')
        putchar('\n');
    return c=='\n' ? SUCCESS : FAIL;
}

int skip_to_body() {
    int c;
    int found_body = FALSE;
    while( (c=getchar()) != EOF && !found_body) {
        putchar(c);
        if(c == '\r' && (c =getchar()) == '\n') {
            printf("\r\n");
            found_body = TRUE;
        }
        else
            skip_to_new_line();
    }
    return found_body;
}

int print_normal_replacement(char * replace_text) {
    printf(replace_text);
}