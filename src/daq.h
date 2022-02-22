#ifndef DAQ_H
#define DAQ_H

#include <stdint.h>

typedef struct daq_t daq_t;

int daq_init(daq_t *daq, uint16_t card_type, uint16_t card_num);
void daq_destroy(daq_t *daq);
int daq_acquire(daq_t *daq, uint16_t *buffer,
                uint32_t samples_per_trig, uint32_t trig_count);

#endif // DAQ_H
