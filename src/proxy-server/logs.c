#include <time.h>

#define TIME_BUFFER_SIZE 20

#include "include/logs.h"

void log_message(bool error, char * message){
    char time_buffer[TIME_BUFFER_SIZE];
    struct tm *tm;
    time_t current_time;

    current_time = time(NULL);
    tm = localtime(&current_time);
    strftime(time_buffer, TIME_BUFFER_SIZE, "%F %T", tm);

    fprintf(stdout, "[%s] %s\n", time_buffer, message);
}