#ifndef STRIPMIME_TYPES_H
#define STRIPMIME_TYPES_H

#define MAX_CONTENT_TYPE 127
#define MAX_BOUNDARY_TYPE 70

typedef struct content_type_header * content_type_header_t;

typedef struct content_type_header {
    char content_type[MAX_CONTENT_TYPE];
    char boundary[MAX_BOUNDARY_TYPE];
} content_type_header;

#endif