#ifndef EXTERNAL_TRANSFORMATIONS_H
#define EXTERNAL_TRANSFORMATIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#define WORKING 1
#define FINISHED 0
#define TRUE 1
#define FALSE 0
#define FINISH_LENGTH 5
#define FINISH_STRING "\n\r.\n\r"

// function that extract body from pop3 message recived by server ( extract first line )
char * extract_body(char * buffer, int buffer_size);
char * transform(char * transform_command , char * buffer, int buffer_size);
char * text_to_pop3(char * buffer, int buffer_size);
char * pop3_to_text(char * buffer, int buffer_size);

#endif
