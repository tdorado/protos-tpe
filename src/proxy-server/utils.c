#include <stdio.h>
#include <string.h>

#include "include/utils.h"

#define MAX(n1, n2) ( (n1) > (n2) ? (n1) : (n2) )

int max_of_five(int n1, int n2, int n3, int n4, int n5) {
    return MAX(MAX(MAX(MAX(n1, n2), n3), n4), n5);
}

int max_of_three(int n1, int n2, int n3){
    return MAX(MAX(n1, n2), n3);
}

ssize_t read_from_fd(int *fd, buffer_t buffer) {
    uint8_t *ptr;
    ssize_t n;
    size_t count = 0;

    ptr = buffer_write_ptr(buffer, &count);
    n = read(*fd, ptr, count);
    buffer_write_adv(buffer, n);

    return n;
}

ssize_t write_to_fd(int *fd, buffer_t buffer) {
    uint8_t *ptr;
    ssize_t n;
    size_t count = 0;

    ptr = buffer_read_ptr(buffer, &count);
    n = write(*fd, ptr, count);
    buffer_read_adv(buffer, n);

    return n;
}

ssize_t send_capa_message(const int *fd) {
    return write(*fd, "CAPA\r\n", 6);
}

ssize_t send_message_to_fd(const int *fd, const char *message, const size_t messageLength) {
    return write(*fd, message, messageLength);
}