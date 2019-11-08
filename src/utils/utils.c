#include "include/utils.h"

#define MAX(n1, n2) ( (n1) > (n2) ? (n1) : (n2) )

int max(int n1, int n2, int n3, int n4, int n5) {
    return MAX(MAX(MAX(MAX(n1, n2), n3), n4), n5);
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

void printf_buffer(buffer_t buffer) {
    buffer_t new_buffer = initialize_buffer(BUFFER_SIZE);
    uint8_t character;

    while (buffer_can_read(buffer)) {
        character = buffer_read(buffer);
        buffer_write(new_buffer, character);
        printf("%c", character);
    }

    while (buffer_can_read(new_buffer)) {
        buffer_write(buffer, buffer_read(new_buffer));
    }

    free(new_buffer->data);
    free(new_buffer);

    printf("\n");
}

void remove_pop_first_line(buffer_t buffer, buffer_t pop_message) {
    uint8_t character;
    /* Saco la primer linea del pop3 */
    while (buffer_can_read(buffer) && (character = buffer_read(buffer)) != '\r') {
        buffer_write(pop_message, character);
    }
    buffer_write(pop_message, buffer_read(buffer));
}

int remove_pop_last_line(buffer_t buffer) {
    /* Saco \r\n.\r\n */
    char *ptr;
    ptr = strstr((char *)buffer->read, "\r\n.\r\n");

    if (ptr == NULL) {
        return 0;
    }

    buffer->write = (uint8_t *)ptr;
    return 1;
}