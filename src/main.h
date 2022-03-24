#ifndef MAIN_H
#define MAIN_H

#include "daq.h"
#include "thread.h"
#include "util.h"
#include "window.h"

#define RADAR_FREQ 94e9
#define RADAR_WAVELENGTH (SPEED_OF_LIGHT / RADAR_FREQ)

typedef struct
{
  size_t chirp_size;       // Number of samples in each chirp
  size_t cpi_size;         // Number of chirps in each CPI
  size_t frame_size;       // Number of CPIs in each frame
  win_type_t window_type;  // Fast and slow time window function

  size_t dopper_range_bin; // Range bin to use for Doppler moment extraction
} proc_options_t;

typedef struct
{
  double chirp_start;    // Chirp start frequency in MHz
  double chirp_end;      // Chirp end frequency in MHz
  double chirp_duration; // Chirp duration in microseconds
} dds_options_t;

typedef struct
{
  char *path;
} log_options_t;

typedef struct
{
  int channel;          // DAQ channel number
  double sampling_rate; // Sampling rate of ADC (MHz)

  int continuous;
} daq_options_t;

typedef struct
{
  proc_options_t proc; // FMCW processing options
  dds_options_t dds;   // DDS options
  log_options_t log;   // Logging options
  daq_options_t daq;   // DAQ options
} options_t;

extern options_t default_options;

#endif
