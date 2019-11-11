#include <stdio.h>
#include <stdlib.h>

#define CONTENT_TYPE "Content-Type: "
#define CONTENT_TYPE_LENGTH 14
#define MAX_CONTENT_TYPE 127
#define MAX_BOUNDARY_TYPE 70
#define HEADER_TO_BODY_STRING "\r\n\r\n"
#define SUCCESS 1
#define FAIL -1
#define TRUE 1
#define FALSE 0


int headers();
int skip_line();
int skip_to_body();

typedef struct content_type_header {
    char content_type[MAX_CONTENT_TYPE];
    char boundary[MAX_BOUNDARY_TYPE];
} content_type_header;

typedef struct content_type_header * content_type_header_t;

int main(void) {
    content_type_header_t content_type = malloc(sizeof(content_type_header));
    headers(content_type);
    printf("%s\n", content_type->content_type);
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
                if( c == '\r' && (c = getchar()) == '\n') {
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