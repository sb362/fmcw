#include "ringbuffer.h"

#include "util.h"

#include <assert.h>

struct ring_buffer_t
{
  size_t head, tail, capacity, buf_size, mem_size;
  double *mem;
};

ring_buffer_t *ring_buffer_init(size_t capacity, size_t buf_size)
{
  ring_buffer_t *rbuf = safe_malloc(sizeof(ring_buffer_t));

  rbuf->capacity = capacity;
  rbuf->buf_size = buf_size;
  rbuf->mem = safe_malloc(capacity * buf_size * sizeof(double));

  ring_buffer_clear(rbuf);
  return rbuf;
}

void ring_buffer_free(ring_buffer_t *rbuf)
{
  assert(rbuf);

  aligned_free(rbuf->mem);
}

void ring_buffer_clear(ring_buffer_t *rbuf)
{
  assert(rbuf && rbuf->mem);

  memset(rbuf->mem, 0, rbuf->capacity * rbuf->buf_size * sizeof(double));
  rbuf->head = rbuf->tail = 0;
}

void ring_buffer_write(ring_buffer_t *rbuf, double *buf, size_t buf_size)
{
  assert(rbuf && rbuf->mem);
  assert(rbuf->buf_size == buf_size);

  memcpy(rbuf->mem + rbuf->head * buf_size, buf, buf_size * sizeof(double));

  rbuf->head = (rbuf->head + 1) % rbuf->capacity;
}

double *ring_buffer_read(ring_buffer_t *rbuf)
{
  double *buf = ring_buffer_peek(rbuf);

  rbuf->tail = (rbuf->tail + 1) % rbuf->capacity;

  return buf;
}

double *ring_buffer_peek(ring_buffer_t *rbuf)
{
  return ring_buffer_peek_back(rbuf, 0);
}

double *ring_buffer_peek_back(ring_buffer_t *rbuf, int n)
{
  assert(rbuf && rbuf->mem);
  assert(n < ring_buffer_capacity(rbuf));

  size_t idx = rbuf->tail - (n <= rbuf->tail ? n
                                             : rbuf->capacity + n);

  return rbuf->mem + idx * rbuf->buf_size;
}

size_t ring_buffer_size(ring_buffer_t *rbuf)
{
  assert(rbuf);

  return rbuf->head >= rbuf->tail ?                  rbuf->head - rbuf->tail
                                  : rbuf->capacity - rbuf->tail + rbuf->head;
}

size_t ring_buffer_capacity(ring_buffer_t *rbuf)
{
  assert(rbuf);

  return rbuf->capacity;
}
