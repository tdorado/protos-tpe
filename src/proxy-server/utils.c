#include <stdio.h>
#include <string.h>

#include "include/utils.h"

#define MAX(n1, n2) ( (n1) > (n2) ? (n1) : (n2) )

int max_of_five(int n1, int n2, int n3, int n4, int n5) {
    return MAX(MAX(MAX(MAX(n1, n2), n3), n4), n5);
}

int max_of_three(int n1, int n2, int n3) {
    return MAX(MAX(n1, n2), n3);
}

ssize_t read_from_fd(int fd, buffer_t buffer) {
    uint8_t * ptr;
    size_t len;
    ssize_t ret = -1;

    ptr = buffer_write_ptr(buffer, &len);
    ret = read(fd, ptr, len);
    if(ret > 0){
        buffer_write_adv(buffer, ret);
    }

    return ret;
}

ssize_t write_to_fd(int fd, buffer_t buffer) {
    uint8_t * ptr;
    size_t len;
    ssize_t ret;

    ptr = buffer_read_ptr(buffer, &len);
    ret = write(fd, ptr, len);
    if (ret > 0){
        buffer_read_adv(buffer, ret);
    }

    return ret;
}

ssize_t send_message_to_fd(const int fd, const char *message, const size_t messageLength) {
    return write(fd, message, messageLength);
}

ssize_t write_until_enter_to_fd(int fd, buffer_t buffer) {
    uint8_t * ptr;
    size_t len;
    ssize_t ret = 0;

    ptr = buffer_read_ptr(buffer, &len);
    len--;
    while( ptr[ret] != '\n' && (size_t)ret < len) {
        ret++;
    }
    ret++;

    ret = write(fd, ptr, ret);
    if(ret > 0){
        buffer_read_adv(buffer, ret);
    }

    return ret;
}
