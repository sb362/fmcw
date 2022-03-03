#include "ringbuffer.h"

struct ring_buffer_t
{
	size_t head, tail;
	double *buffer;
};

