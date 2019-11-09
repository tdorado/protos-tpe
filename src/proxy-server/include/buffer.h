#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

typedef struct buffer * buffer_t;

struct buffer
{
    uint8_t *data;

    /** límite superior del buffer. inmutable */
    uint8_t *limit;

    /** puntero de lectura */
    uint8_t *read;

    /** puntero de escritura */
    uint8_t *write;
};

/**
 * inicializa el buffer sin utilizar el heap
 */
void buffer_init(buffer_t b, const size_t n, uint8_t *data);

/**
 * Retorna un puntero donde se pueden escribir hasta `*nbytes`.
 * Se debe notificar mediante la función `buffer_write_adv'
 */
uint8_t * buffer_write_ptr(buffer_t b, size_t *nbyte);
void buffer_write_adv(buffer_t b, const ssize_t bytes);

uint8_t * buffer_read_ptr(buffer_t b, size_t *nbyte);
void buffer_read_adv(buffer_t b, const ssize_t bytes);

/**
 * obtiene un byte
 */
uint8_t buffer_read(buffer_t b);

/** escribe un byte */
void buffer_write(buffer_t b, uint8_t c);

/**
 * compacta el buffer
 */
void buffer_compact(buffer_t b);

/**
 * Reinicia todos los punteros
 */
void buffer_reset(buffer_t b);

/** retorna true si hay bytes para leer del buffer */
bool buffer_can_read(buffer_t b);

/** retorna true si se pueden escribir bytes en el buffer */
bool buffer_can_write(buffer_t b);

buffer_t init_buffer(const size_t bytes);

void buffer_move(buffer_t src, buffer_t dst);

void free_buffer(buffer_t buffer);

void print_buffer(buffer_t buffer);

#endif
