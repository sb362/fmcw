#ifndef DDS_H
#define DDS_H

#include <stdint.h>

#define DRG_NO_DWELL_LOW 1
#define DRG_NO_DWELL_HIGH 2
#define DRG_NO_DWELL 3

int dds_write_bytes(uint8_t address, int count, uint8_t* data);
int dds_write_uint8(uint8_t address, uint8_t param);
int dds_write_uint16(uint8_t address, uint16_t param);
int dds_write_uint16_pair(uint8_t address, uint16_t upper, uint16_t lower);
int dds_write_uint32(uint8_t address, uint32_t param);
int dds_write_uint32_pair(uint8_t address, uint32_t upper, uint32_t lower);
int dds_write_uint64(uint8_t address, uint64_t param);

int dds_read_bytes(uint8_t address, int count, uint8_t* data);
int dds_read_uint8(uint8_t address, uint8_t* param);
int dds_read_uint16(uint8_t address, uint16_t* param);
int dds_read_uint16_pair(uint8_t address, uint16_t* upper, uint16_t* lower);
int dds_read_uint32(uint8_t address, uint32_t* param);
int dds_read_uint32_pair(uint8_t address, uint32_t* upper, uint32_t* lower);
int dds_read_uint64(uint8_t address, uint64_t* param);

int dds_reset(void);
int dds_update(void);
int dds_powerup(void);
int dds_powerdown(void);
int dds_hello(void);

int ad9914_calibrate_dac(void);
int ad9914_single_tone(double frequency, double* actual_frequency);
int ad9914_ramp_generator(double start_freq, double stop_freq, double period, uint32_t flags, double* actual_start, double* actual_stop, double* actual_period);

int init_dds(void);
int deinit_dds(void);

#endif // DDS_H
