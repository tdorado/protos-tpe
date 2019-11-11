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
#define START_BOUNDARY 1
#define FINAL_BOUNDARY 2


typedef struct content_type_header {
    char content_type[MAX_CONTENT_TYPE];
    char boundary[MAX_BOUNDARY_TYPE];
    int boundary_length;
} content_type_header;

typedef struct content_type_header * content_type_header_t;

int headers();
int skip_line();
int skip_to_body();
int handle_attributes(content_type_header_t);
int manage_multipart(content_type_header_t, char *, char *);
int search_boundary(char *, int );

int main(void) {
    content_type_header_t content_type = malloc(sizeof(content_type_header));
    headers(content_type);
    if(strcmp(content_type->content_type, "multipart/")) {
        manage_multipart(content_type, "text/plain", "Confiscado");
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
    int boundary_length = scanf(" boundary=%s", content_type->boundary);
    content_type->boundary_length = boundary_length;
    printf(" boundary=%s\r\n", content_type->boundary);
    getchar();
    getchar();
}

int search_boundary(char * boundary, int print) {
    int c;
    int boundary_position = 0;
    while( (c = getchar()) != EOF ) {
        switch(c) {
            case '-' :
                if((c = getchar()) == '-') {
                    while( (c = getchar()) == boundary[boundary_position])
                        boundary_position++;
                    if( c == '\r' && boundary[boundary_position] == '\0') {
                        getchar();
                        printf("--%s\r\n", boundary);
                        return START_BOUNDARY;
                    }
                    else if( c == '-' && (c=getchar()) == '-' && boundary[boundary_position] == '\0'){
                        getchar();
                        getchar();
                        printf("--%s--\r\n", boundary);
                        return FINAL_BOUNDARY;
                    }
                    else  {
                        if(print) printf("--");
                        for(int i = 0; i<boundary_position; i++){
                            putchar(boundary[i]);
                        }
                        boundary_position=0;
                    }
                }
                else
                    if(print) printf("-%c", c);
                break;
            default:
                if(print) putchar(c);
                boundary_position = 0;
        }
    }
    return FAIL;
}

int manage_multipart(content_type_header_t content_type, char * replace_mime, char * replace_text) {
    content_type_header_t new_content_type = malloc(sizeof(content_type_header));
    int response;
    int print = TRUE;
    while((response = search_boundary(content_type->boundary, print)) != EOF) {
        if(response == START_BOUNDARY) {
            headers(new_content_type);
            if(strncmp(new_content_type->content_type, "multipart/", 10) == 0) {
                manage_multipart(new_content_type, replace_mime, replace_text);
            }
            else if(strcmp(new_content_type->content_type, replace_mime) == 0) {
                printf("%s\r\n", replace_text);
                print = FALSE;
            }
            else {
                print = TRUE;
            }
        }
        else if(response == FINAL_BOUNDARY) {
            break;
        }
    }
    free(new_content_type);
}
