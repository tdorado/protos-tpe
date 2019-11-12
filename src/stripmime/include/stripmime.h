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
int skip_line(void);
int skip_to_body(void);
int handle_attributes(content_type_header_t content_type);
int manage_body(stack_t stack, char * replace_mime, char * replace_text);
int search_boundary(char * boundary, int print);
void check_variables(char ** filter_medias, char ** filter_msg);
int contains_string(char * string, char * string_array);

#endif
