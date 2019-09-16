#ifndef EXTERNAL_TRANSFORMATIONS_H
#define EXTERNAL_TRANSFORMATIONS_H

// function that extract body from pop3 message recived by server ( extract first line )
char * extract_body(char * buffer, int buffer_size);
char * transform(char * transform_command , char * email);

#endif