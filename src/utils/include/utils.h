#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>

#include "buffer.h"

#define BUFFER_SIZE 2048

int max(int n1, int n2, int n3, int n4, int n5);
ssize_t read_from_fd(int *fd, buffer_t buffer);
ssize_t write_to_fd(int *fd, buffer_t buffer);
ssize_t send_capa_message(const int *fd);
ssize_t send_message_to_fd(const int *fd, const char *message, const size_t messageLength);
void printf_buffer(buffer_t buffer);
void remove_pop_first_line(buffer_t buffer, buffer_t pop_message);
int remove_pop_last_line(buffer_t buffer);

#endif