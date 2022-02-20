#ifndef FMCW_H
#define FMCW_H

#include <fftw3.h>

// Coherent processing interval
typedef struct fmcw_cpi fmcw_cpi_t;
struct fmcw_cpi
{
  size_t chirp_size,  // Number of samples in each chirp
         cpi_size,    // Number of chirps in each CPI
         buffer_size; // Number of samples in each CPI

  double       *volts;             // Voltages calculated from ADC integers
  fftw_complex *freq_spectrum;     // Result of fast-time FFT
  double       *freq_spectrum_db;  // Fast-time FFT magnitude in decibel scale
  fftw_complex *range_doppler;     // Result of slow-time FFT
  double       *range_doppler_db;  // Slow-time FFT magnitude in decibel scale

  double       *velocity_spectrum; //
};

typedef struct fmcw_context fmcw_context_t;
struct fmcw_context
{
  fftw_plan fast_time, slow_time;
  fmcw_cpi_t cpi;
};

#endif
