#ifndef MAIN_H
#define MAIN_H

// Fix for missing type _int64
#ifdef _CVI_
#define __MINGW32__
#endif
#include <pthread.h>
#ifdef _CVI_
#undef __MINGW32__
#endif

#include "daq.h"
#include "window.h"

typedef struct
{
  size_t chirp_size, cpi_size;
  win_type_t window_type;

  double avg_time;      // Averaging time in milliseconds
} proc_options_t;

typedef struct
{
  double chirp_start,    // Chirp start frequency in MHz
         chirp_end,      // Chirp end frequency in MHz
         chirp_duration; // Chirp duration in microseconds
} dds_options_t;

typedef struct
{
  char *path;
} log_options_t;

typedef struct
{
  proc_options_t proc;
  dds_options_t dds;
  log_options_t log;

  int channel;
} options_t;

void start_main_thread(daq_t *daq, options_t *options);
void stop_main_thread();

#endif
