#ifndef LOGS_H
#define LOGS_H

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define TIME_BUFFER_SIZE 20

void log_message(bool error, char * message);

#endif
