#include "include/stripmime.h"

int main(void) {
    content_type_header_t content_type = malloc(sizeof(content_type_header));
    stack_t stack = create_stack();
    manage_body(stack, "hola/chau", "Confiscado");
    free(stack);
    free(content_type);
}

int headers(content_type_header_t content_type, char * replace_mime) {
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
                    content_type->content_type[content_actual_length] = c;
                    content_actual_length++;
                }
                content_type->content_type[content_actual_length] = '\0';
                if( c == ';') {
                    if (strcmp(content_type->content_type, replace_mime) == 0) {
                        printf("text/plain");
                    }
                    else
                        printf("%s;", content_type->content_type);
                    handle_attributes(content_type);
                    skip_to_body();
                    return SUCCESS;
                }
                else if( c == '\r' && (c = getchar()) == '\n') {
                    if (strcmp(content_type->content_type, replace_mime) == 0) {
                        printf("text/plain\r\n");
                    }
                    else
                        printf("%s\r\n", content_type->content_type);
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
    fprintf(stderr, "Bad headers formats");
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

int manage_body(stack_t stack, char * replace_mime, char * replace_text) {
    content_type_header_t actual_content = malloc(sizeof(content_type_header));
    headers(actual_content, replace_mime);
    stack_push(stack, actual_content);
    int print = TRUE;
    int rta;
    while(!stack_is_empty(stack)) {
        actual_content = stack_pop(stack);
        content_type_header_t aux = malloc(sizeof(content_type_header));
        if(strncmp(actual_content->content_type, "multipart/", 10) == 0) {
            rta = search_boundary(actual_content->boundary, print);
            if(rta == START_BOUNDARY) {
                headers(aux, replace_mime);
                stack_push(stack, actual_content);
                stack_push(stack, aux);
            }
            print = TRUE;
        }
        else if(strcmp(actual_content->content_type, replace_mime) == 0) {
            print = FALSE;
            printf("%s\n", replace_text);
        }
        else {
            print = TRUE;
        }
   }
   if(rta != FAIL) {
       int c;
       while((c=getchar()) != EOF)
        putchar(c);
   }
}
