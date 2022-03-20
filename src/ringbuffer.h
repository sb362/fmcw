#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>

typedef struct ring_buffer_t ring_buffer_t;

ring_buffer_t *ring_buffer_init(size_t capacity, size_t buf_size);
void ring_buffer_free(ring_buffer_t *rbuf);
void ring_buffer_clear(ring_buffer_t *rbuf);

void ring_buffer_write(ring_buffer_t *rbuf, double *buf, size_t bsize);
double *ring_buffer_read(ring_buffer_t *rbuf);
double *ring_buffer_peek(ring_buffer_t *rbuf);
double *ring_buffer_peek_back(ring_buffer_t *rbuf, int n);

size_t ring_buffer_size(ring_buffer_t *rbuf);
size_t ring_buffer_capacity(ring_buffer_t *rbuf);

#endif // RING_BUFFER_H
