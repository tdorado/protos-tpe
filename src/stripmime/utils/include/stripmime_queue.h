#ifndef STRIPMIME_QUEUE_H
#define STRIPMIME_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "./stripmime_types.h"

typedef struct node * node_t;
typedef struct stack * stack_t;

#define TRUE 1
#define FALSE 0

stack_t create_stack(void);
void stack_push(stack_t stack, content_type_header_t elem);
content_type_header_t stack_pop(stack_t stack);
content_type_header_t stack_peek(stack_t stack);
int stack_is_empty(stack_t stack);
void stack_free_queue_elems(stack_t stack);

#endif
