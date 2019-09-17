#ifndef EXTERNAL_TRANSFORMATIONS_H
#define EXTERNAL_TRANSFORMATIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define FINISHED 0
#define WORKING 1
#define FALSE 0
#define TRUE 1
#define FINISH_LENGTH 5
#define FINISH_STRING "\n\r.\n\r"

// function that extract body and head from pop3 message received by server
int extract_pop3_info(char * buffer, int buffer_size, char * head, char * body);

//
char * external_transformation(char * transform_command , char * buffer, int buffer_size);

//
int text_to_pop3(char * buffer, int buffer_size, char * pop3_text);

//
int pop3_to_text(char * buffer, int buffer_size, char * text);

//
int call_command(char * command, char * text, int buffer_size, char * transformed_text);

#endif
