#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/buffer.h"

inline void buffer_reset(buffer_t b) {
    b->read = b->data;
    b->write = b->data;
}

void buffer_init(buffer_t b, const size_t n, uint8_t * data) {
    b->data = data;
    buffer_reset(b);
    b->limit = b->data + n;
}

inline bool buffer_can_write(buffer_t b) {
    return b->limit - b->write > 0;
}

inline uint8_t * buffer_write_ptr(buffer_t b, size_t * nbyte) {
    assert(b->write <= b->limit);
    *nbyte = b->limit - b->write;
    return b->write;
}

inline bool buffer_can_read(buffer_t b) {
    return b->write - b->read > 0;
}

inline uint8_t * buffer_read_ptr(buffer_t b, size_t * nbyte) {
    assert(b->read <= b->write);
    *nbyte = b->write - b->read;
    return b->read;
}

inline void buffer_write_adv(buffer_t b, const ssize_t bytes) {
    if (bytes > -1) {
        b->write += (size_t)bytes;
        assert(b->write <= b->limit);
    }
}

inline void buffer_read_adv(buffer_t b, const ssize_t bytes) {
    if (bytes > -1) {
        b->read += (size_t)bytes;
        assert(b->read <= b->write);

        if (b->read == b->write) {
            // compactacion poco costosa
            buffer_compact(b);
        }
    }
}

inline uint8_t buffer_read(buffer_t b) {
    uint8_t ret;
    if (buffer_can_read(b)) {
        ret = *b->read;
        buffer_read_adv(b, 1);
    } else {
        ret = 0;
    }
    return ret;
}

inline void buffer_write(buffer_t b, uint8_t c) {
    if (buffer_can_write(b)) {
        *b->write = c;
        buffer_write_adv(b, 1);
    }
}

void buffer_compact(buffer_t b) {
    if (b->data == b->read) {
        // nada por hacer
    } else if (b->read == b->write) {
        b->read = b->data;
        b->write = b->data;
    } else {
        const size_t n = b->write - b->read;
        memmove(b->data, b->read, n);
        b->read = b->data;
        b->write = b->data + n;
    }
}

buffer_t init_buffer(const size_t bytes) {
    uint8_t * data = (uint8_t *)malloc(bytes*sizeof(uint8_t));
    buffer_t buffer = (buffer_t)malloc(sizeof(*buffer));

    if (data == NULL || buffer == NULL) {
        perror("Error initializing a buffer");
        exit(EXIT_FAILURE);
    }

    buffer_init(buffer, bytes, data);

    // is this necesary?
    // free(data);

    return buffer;
}

void buffer_copy(buffer_t src, buffer_t dst) {
    while (buffer_can_read(src) && buffer_can_write(dst)) {
        buffer_write(dst, buffer_read(src));
    }
}

void free_buffer(buffer_t buffer) {
    free(buffer->data);
    free(buffer);
}


void print_buffer(buffer_t buffer) {
    uint8_t * ptr = buffer->read;

    while(ptr < buffer->write) {
        putchar(*(ptr++));
    }

    putchar('\n');
}
