#ifndef FMCW_H
#define FMCW_H

#include <fftw3.h>

#include "window.h"

// Coherent processing interval
typedef struct fmcw_cpi fmcw_cpi_t;
struct fmcw_cpi
{
  size_t chirp_size,   // Number of samples in each chirp
         cpi_size,     // Number of chirps in each CPI
         buffer_size,  // Total number of samples in each CPI
         n_bins,       // Number of range bins (= chirp_size / 2 + 1)
         fbuffer_size; // = num_bins * cpi_size

  double       *volts;              // Voltages calculated from ADC integers
  fftw_complex *freq_spectrum;      // Result of fast-time FFT
  double       *power_spectrum_dbm; // Fast-time FFT magnitude relative to 1 mW
  fftw_complex *range_doppler;      // Result of slow-time FFT
  double       *range_doppler_dbm;  // Slow-time FFT magnitude relative to 1 mW

  double       *velocity_spectrum;  //
};

typedef struct fmcw_context fmcw_context_t;
struct fmcw_context
{
  fftw_plan fast_time, slow_time;
  win_table_t fast_win, slow_win;
  fmcw_cpi_t cpi;
};

void fmcw_cpi_init(fmcw_cpi_t *cpi, size_t chirp_size, size_t cpi_size);
void fmcw_cpi_destroy(fmcw_cpi_t *cpi);
void fmcw_context_init(fmcw_context_t *ctx, size_t chirp_size,
                       size_t cpi_size, win_type_t win_type);
void fmcw_context_destroy(fmcw_context_t *ctx);
void fmcw_process(fmcw_context_t *ctx);

#endif // FMCW_H
