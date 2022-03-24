#ifndef DAQ_H
#define DAQ_H

#include <stdint.h>
#include <stdbool.h>

typedef struct daq_t daq_t;

daq_t *daq_init(uint16_t card_type, uint16_t card_num);
void daq_destroy(daq_t *daq);

// daq_acquire() initiates the acquisition of *trig_count* triggers each
// containing *samples_per_trig* samples. If *async* is true, then
// this is an asynchronous operation and you should use daq_await()
// to test if the acquisition is complete. If *async* is false, then this
// function will block until the acquisition is complete.
// *buffer* should be allocated by aligned_malloc().
// Returns a positive integer indicating how many samples were acquired,
// or a negative integer if the acquisition failed.
int daq_acquire(daq_t *daq,
                uint16_t channel,
                uint16_t *buffer,
                uint32_t samples_per_trig,
                uint32_t trig_count,
                bool async);

// Blocks until the previous asynchronous input operation is complete.
void daq_await(daq_t *daq);

// Returns true if the previous asynchronous input operation is complete.
int daq_ready(daq_t *daq);

#ifdef FAKE_DAQ
#include "Wd-dask64.h"
#include "wddaskex.h"

#define DAQ_DEFAULT_CARD_TYPE PCI_9846D
#else
#define DAQ_DEFAULT_CARD_TYPE 0
#endif

#endif // DAQ_H
