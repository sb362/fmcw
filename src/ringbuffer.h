#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>

typedef struct ring_buffer_t ring_buffer_t;

void ring_buffer_init(ring_buffer_t *buffer, size_t size);
void ring_buffer_free(ring_buffer_t *buffer);
void ring_buffer_clear(ring_buffer_t *buffer);



#endif // RING_BUFFER_H
