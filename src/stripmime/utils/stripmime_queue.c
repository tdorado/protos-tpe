#include "include/stripmime_queue.h"

typedef struct node {
    content_type_header_t elem;
    node_t next;
} node ;

typedef struct stack {
    node_t head;
} stack;

stack_t create_stack() {
    stack_t stack = malloc(sizeof(stack));
    stack->head = NULL;
    return stack;
}

void stack_push(stack_t stack, content_type_header_t elem) {
    node_t node = malloc(sizeof(node));
    node->elem = elem;
    node->next = stack->head;
    stack->head = node;
}

content_type_header_t stack_pop(stack_t stack) {
    if(stack->head != NULL) {
        node_t aux = stack->head;
        content_type_header_t elem = aux->elem;
        stack->head = stack->head->next;
        free(aux);
        return elem;
    }
    return NULL;
}

content_type_header_t stack_peek(stack_t stack) {
    if(stack->head != NULL) {
        return stack->head->elem;
    }
    return NULL;
}

int stack_is_empty(stack_t stack) {
    return stack->head == NULL ? TRUE : FALSE;
}