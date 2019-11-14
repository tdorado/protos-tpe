#include <stdio.h>
#include <stdlib.h>

#include "include/error_file.h"

void redirect_stderr(settings_t settings) {
    FILE * f = freopen(settings->error_file, "a+", stderr);

    if (f == NULL) {
        exit(EXIT_FAILURE);
    }
}
