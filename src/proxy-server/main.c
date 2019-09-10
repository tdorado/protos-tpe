#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "include/input_parser.h"

int main(int argc, char ** argv) {

    input_t proxy_params = malloc(sizeof(struct input_params));
    
    if (input_parser(argc, argv, proxy_params) < 0) {
        free(proxy_params);
        return 1;
    }

    return 0;
}