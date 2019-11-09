#ifndef UTILS_H
#define UTILS_H

#include "buffer.h"

int max_of_five(int n1, int n2, int n3, int n4, int n5);
int max_of_three(int n1, int n2, int n3);
ssize_t read_from_fd(int *fd, buffer_t buffer);
ssize_t write_to_fd(int *fd, buffer_t buffer);
ssize_t write_until_enter_to_fd(int *fd, buffer_t buffer);
ssize_t send_capa_message(const int *fd);
ssize_t send_message_to_fd(const int *fd, const char *message, const size_t messageLength);

#endif