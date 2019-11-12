#include "include/stripmime.h"

const char * env_variables[]  = {"FILTER_MEDIAS", "FILTER_MSG"};

int main(void) {
    char * filter_medias;
    char * filter_msg;
    if( check_variables(&filter_medias, &filter_msg) == FAIL ) {
        print_all_stdin();
        return FAIL;
    }
    stack_t stack = create_stack();
    manage_body(stack, filter_medias, filter_msg);
    stack_free_queue_elems(stack);
}

int check_variables(char ** filter_medias, char ** filter_msg) {
    char * aux = getenv(env_variables[0]);
    if(aux == NULL) {
        fprintf(stderr, "La variable FILTER_MEDIAS no está definida");
        return FAIL;
    } else {
        *filter_medias = aux;
        aux = getenv(env_variables[1]);
        if (aux == NULL) {
            *filter_msg = FILTER_MSG_DEFAULT;
        } else {
            *filter_msg = aux;
        }
    }
    return SUCCESS;
}

int contains_string(char * string, char * string_array) {
    int i = 0;
    int j = 0;
    while(string_array[i] != '\0') {
        while( string[j] == string_array[i] && (string[j] != '\0' && string_array[i] != '\0')) {
            i++;
            j++;
            if(string_array[i] == '/' && string_array[i+1] == '*') {
                return TRUE;
            }
        }
        if( string[j] == '\0' && (string_array[i] == '\0' || string_array[i] == ',')) {
            return TRUE;
        }
        while(string_array[i] != ',' && string_array[i] != '\0') {
            i++;
        }
        j=0;
        if(string_array[i] == ',') {
            i++;
        }
    }
    return FALSE;
}

int headers(content_type_header_t content_type, char * replace_mime) {
    int c;
    int content_length = 0;
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
                    printf("%s;", content_type->content_type);
                    handle_attributes(content_type);
                    skip_to_body();
                    return SUCCESS;
                }
                else if( c == '\r' && (c = getchar()) == '\n') {
                    printf("%s\r\n", content_type->content_type);
                    skip_to_body();
                    return SUCCESS;
                }
            }
        } else {
            content_length = 0;
            skip_line();
        }
    }
    return FAIL;
}

int skip_line(void) {
    int c;
    while((c = getchar()) != EOF ) {
        if( c == '\r' && (c=getchar()) == '\n') {
            printf("\r\n");
            return SUCCESS;
        } else {
            putchar(c);
        }
    }
    fprintf(stderr, "Bad headers formats");
    return FAIL;
}

int skip_to_body(void) {
    int c;
    while( ( c = getchar()) != EOF) {
        if ( c == '\r' && (c = getchar()) == '\n') {
            printf("\r\n");
            return SUCCESS;
        } else {
            putchar(c);
            skip_line();
        }
    }
    fprintf(stderr, "Bad headers format");
    return FAIL;
}

int handle_attributes(content_type_header_t content_type) {
    int rta = scanf(" boundary=\"%s", content_type->boundary);
    if(rta == 1) {
        int i = 0;
        while(content_type->boundary[i] != '\0') {
            i++;
        }
        content_type->boundary[i-1] = '\0';
        printf(" boundary=\"%s\"\r\n", content_type->boundary);
        getchar();
        getchar();
        return SUCCESS;
    } else {
        putchar(' ');
        skip_line();
    }
    return FAIL;
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
                    } else if( c == '-' && (c=getchar()) == '-' && boundary[boundary_position] == '\0') {
                        getchar();
                        getchar();
                        printf("--%s--\r\n", boundary);
                        return FINAL_BOUNDARY;
                    } else  {
                        if(print) printf("--");
                        for(int i = 0; i<boundary_position; i++) {
                            putchar(boundary[i]);
                        }
                        boundary_position=0;
                    }
                } else {
                    if(print) printf("-%c", c);
                }
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
        if(strncmp("multipart/", actual_content->content_type, 10) == 0) {
            rta = search_boundary(actual_content->boundary, print);
            if(rta == START_BOUNDARY) {
                content_type_header_t aux = malloc(sizeof(content_type_header));
                headers(aux, replace_mime);
                stack_push(stack, actual_content);
                stack_push(stack, aux);
            }
            print = TRUE;
        } else if(strncmp("message/", actual_content->content_type, 8) == 0) {
            stack_t stack = create_stack();
            manage_body(stack, replace_mime, replace_text);
            stack_free_queue_elems(stack);
            free(actual_content);
        }
        else if(contains_string(actual_content->content_type, replace_mime)) {
            print = FALSE;
            printf("%s\r\n", replace_text);
            free(actual_content);
        } else {
            print = TRUE;
            free(actual_content);
        }
   }
   if(rta != FAIL) {
        int c;
        while((c=getchar()) != EOF)
        putchar(c);
   }
   free(actual_content);
   return SUCCESS;
}

void print_all_stdin(void) {
    int c;
    while((c=getchar()) != EOF) {
        putchar(c);
    }
}