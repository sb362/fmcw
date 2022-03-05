#ifndef MAIN_H
#define MAIN_H

#include "window.h"

typedef struct
{
  size_t chirp_size,      // Number of samples in each chirp
         cpi_size,        // Number of chirps in each CPI
         frame_size;      // Number of CPIs in each frame
  win_type_t window_type;  

} proc_options_t;

typedef struct
{
  double chirp_start,      // Chirp start frequency in MHz
         chirp_end,        // Chirp end frequency in MHz
         chirp_duration;   // Chirp duration in microseconds
} dds_options_t;

typedef struct
{
  char *path;
} log_options_t;

typedef struct
{
  int channel;             // DAQ channel number
} daq_options_t;

typedef struct
{
  proc_options_t proc;
  dds_options_t dds;
  log_options_t log;
  daq_options_t daq;
} options_t;

#endif
