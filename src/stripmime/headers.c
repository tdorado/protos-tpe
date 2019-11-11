#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTENT_TYPE "Content-Type: "
#define CONTENT_TYPE_LENGTH 14
#define MAX_CONTENT_TYPE 127
#define MAX_BOUNDARY_TYPE 70
#define HEADER_TO_BODY_STRING "\r\n\r\n"
#define BOUNDARY "boundary="
#define SUCCESS 1
#define FAIL -1
#define TRUE 1
#define FALSE 0


typedef struct content_type_header {
    char content_type[MAX_CONTENT_TYPE];
    char boundary[MAX_BOUNDARY_TYPE];
} content_type_header;

typedef struct content_type_header * content_type_header_t;

int headers();
int skip_line();
int skip_to_body();
int handle_attributes(content_type_header_t);
int manage_multipart(content_type_header_t, char *);


int main(void) {
    content_type_header_t content_type = malloc(sizeof(content_type_header));
    headers(content_type);
    if(strcmp(content_type->content_type, "multipart/")) {
        manage_multipart(content_type, "text/plain");
    }
    free(content_type);
}

int headers(content_type_header_t content_type) {
    int c;
    int content_length = 0;
    int attribute_length = 0;
    int content_actual_length = 0;
    while((c = getchar()) != EOF) {
        putchar(c);
        if( c == CONTENT_TYPE[content_length]) {
            content_length++;
            if(content_length == CONTENT_TYPE_LENGTH) {
                while( (c = getchar()) != '\r' && c != ';') {
                    putchar(c);
                    content_type->content_type[content_actual_length] = c;
                    content_actual_length++;
                }
                content_type->content_type[content_actual_length] = '\0';
                if( c == ';') {
                    putchar(c);
                    handle_attributes(content_type);
                    skip_to_body();
                    return SUCCESS;
                }
                else if( c == '\r' && (c = getchar()) == '\n') {
                    printf("\r\n");
                    skip_to_body();
                    return SUCCESS;
                }
            }
        }
        else {
            content_length = 0;
            skip_line();
        }
    }
    return FAIL;
}

int skip_line() {
    int c;
    while((c = getchar()) != EOF ) {
        putchar(c);
        if( c == '\r' && (c=getchar()) == '\n') {
            printf("\n\r");
            return SUCCESS;
        }
    }
    fprintf(stderr, "Bad headers format");
    return FAIL;
}

int skip_to_body() {
    int c;
    while( ( c = getchar()) != EOF) {
        putchar(c);
        if ( c == '\r' && (c = getchar()) == '\n') {
            putchar('\n');
            return SUCCESS;
        }
        else {
            skip_line();
        }
    }
    fprintf(stderr, "Bad headers format");
    return FAIL;
}

int handle_attributes(content_type_header_t content_type) {
    int c;
    c = getchar();
    scanf(" boundary=%s", content_type->boundary);
    printf(" boundary=%s\r\n", content_type->boundary);
    getchar();
    getchar();
}

int manage_multipart(content_type_header_t content_type, char * replace_mime) {
    int c;
    char start_boundary[MAX_BOUNDARY_TYPE + 2];
    start_boundary[0] = '\0';
    strcat(start_boundary, "--");
    strcat(start_boundary, content_type->boundary);
    char finish_boundary[MAX_BOUNDARY_TYPE + 4];
    finish_boundary[0] = '\0';
    strcat(finish_boundary, "--");
    strcat(finish_boundary, content_type->boundary);
    strcat(finish_boundary, "--");
    printf("%s\n", start_boundary);
    printf("%s\n", finish_boundary);
}