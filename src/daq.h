#ifndef DAQ_H
#define DAQ_H

#include <stdint.h>
#include <stdbool.h>

typedef struct daq_t daq_t;

daq_t *daq_init(uint16_t card_type, uint16_t card_num);
void daq_destroy(daq_t *daq);

// daq_acquire() blocks until the card has acquired *trig_count* triggers each
// containing *samples_per_trig* samples.
// *buffer* should be allocated by aligned_malloc().
// Returns a positive integer indicating how many samples were acquired,
// or a negative integer if the acquisition failed.
int daq_acquire(daq_t *daq,
                uint16_t *buffer,
                uint32_t samples_per_trig,
                uint32_t trig_count,
                bool async);

int daq_await(daq_t *daq);

#endif // DAQ_H
