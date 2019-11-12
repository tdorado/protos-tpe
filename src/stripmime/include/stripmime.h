#ifndef STRIPMIME_H
#define STRIPMIME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/include/stripmime_types.h"
#include "../utils/include/stripmime_queue.h"

#define CONTENT_TYPE "Content-Type: "
#define CONTENT_TYPE_LENGTH 14
#define HEADER_TO_BODY_STRING "\r\n\r\n"
#define BOUNDARY "boundary="
#define SUCCESS 1
#define FAIL -1
#define TRUE 1
#define FALSE 0
#define START_BOUNDARY 1
#define FINAL_BOUNDARY 2
#define ENV_VARIABLES_QUANTITY 2
#define FILTER_MSG_DEFAULT "Confiscado"
#define MAX_FILTER_MEDIAS 5

int headers(content_type_header_t content_type, char * replace_mime);
int skip_line();
int skip_to_body();
int handle_attributes(content_type_header_t);
int manage_body(stack_t , char *, char *);
int search_boundary(char *, int );
void check_variables(char ** , char **);
int contains_string(char * string, char * string_array);

#endif
