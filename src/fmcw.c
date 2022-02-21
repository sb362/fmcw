#include "fmcw.h"
#include "util.h"
#include "window.h"

#include <math.h>

void fmcw_cpi_init(fmcw_cpi_t *cpi, size_t chirp_size, size_t cpi_size)
{
  LOG_FMT(TRACE, "Initialising CPI (%llu, %llu)", chirp_size, cpi_size);
  cpi->chirp_size      = chirp_size;
  cpi->cpi_size        = cpi_size;
  cpi->buffer_size     = chirp_size * cpi_size;
  cpi->n_bins          = chirp_size / 2 + 1;
  cpi->fbuffer_size    = cpi->n_bins * cpi_size;

  size_t re = sizeof(double), cx = sizeof(fftw_complex);
  cpi->volts              = aligned_malloc(re * cpi-> buffer_size);
  cpi->freq_spectrum      = aligned_malloc(cx * cpi->fbuffer_size);
  cpi->power_spectrum_dbm = aligned_malloc(re * cpi->fbuffer_size);
  cpi->range_doppler      = aligned_malloc(cx * cpi->fbuffer_size);
  cpi->range_doppler_dbm  = aligned_malloc(re * cpi->fbuffer_size);
}

void fmcw_cpi_destroy(fmcw_cpi_t *cpi)
{
  aligned_free(cpi->volts);
  aligned_free(cpi->freq_spectrum);
  aligned_free(cpi->power_spectrum_dbm);
  aligned_free(cpi->range_doppler);
  aligned_free(cpi->range_doppler_dbm);
}

void fmcw_context_init(fmcw_context_t *ctx, size_t chirp_size, size_t cpi_size)
{
  fmcw_cpi_init(&ctx->cpi, chirp_size, cpi_size);

  // Details on fftw_plan_many_dft() are available at:
  // https://www.fftw.org/fftw3_doc/Advanced-Complex-DFTs.html

  LOG(TRACE, "Initialising fast-time FFT plan...");
  int nfast[] = {chirp_size};
  ctx->fast_time = fftw_plan_many_dft_r2c(
    1,        // rank, we are performing only 1D transforms along each row
    nfast,    // length of 1D transforms = number of columns (range bins)
    cpi_size, // number of 1D transforms = number of rows (chirps)
    ctx->cpi.volts,         // input array
    NULL,                   // unused, for FFT'ing subarrays
    1,                      // distance between elements in each input row
    chirp_size,             // distance between each transform input (row)
    ctx->cpi.freq_spectrum, // output array
    NULL,                   // unused, for FFT'ing subarrays
    1,                      // distance between elements in each output row
    chirp_size / 2,         // distance between each transform output (row)
    FFTW_MEASURE            // optimise plan by profiling several FFTs
  );

  LOG(TRACE, "Initialising slow-time FFT plan...");
  int nslow[] = {cpi_size}; 
  ctx->slow_time = fftw_plan_many_dft(
    1,              // rank, we are performing only 1D transforms along each col
    nslow,          // length of 1D transforms = number of rows (chirps)
    chirp_size / 2, // number of 1D transforms = number of columns (range bins)
    ctx->cpi.freq_spectrum, // input array
    NULL,                   // unused, for FFT'ing subarrays
    chirp_size / 2,         // distance between elements in each input column
    1,                      // distance between each transform input (column)
    ctx->cpi.range_doppler, // output array
    NULL,                   // unused, for FFT'ing subarrays
    chirp_size / 2,         // distance between elements in each output column
    1,                      // distance between each transform output (column)
    FFTW_FORWARD,					  // forward transform
    FFTW_MEASURE            // optimise plan by profiling several FFTs
  );

  LOG(TRACE, "Done.");

  ctx->win_type = NO_WINDOW;
}

void fmcw_context_destroy(fmcw_context_t *ctx)
{
  fftw_destroy_plan(ctx->fast_time);
  fftw_destroy_plan(ctx->slow_time);
  fmcw_cpi_destroy(&ctx->cpi);
}

void fmcw_process(fmcw_context_t *ctx)
{
  double dt = elapsed_milliseconds();

  LOG(TRACE, "Fast-time windowing...");
  for (size_t chirp = 0; chirp < ctx->cpi.cpi_size; ++chirp)
    scaled_window(ctx->win_type, ctx->cpi.volts, ctx->cpi.chirp_size, 1);
  
  LOG(TRACE, "Fast-time FFT...");
  fftw_execute(ctx->fast_time);

  LOG(TRACE, "Slow-time windowing...");
  for (size_t chirp = 0; chirp < ctx->cpi.n_bins; ++chirp)
    scaled_window_cx(ctx->win_type, ctx->cpi.freq_spectrum,
                     ctx->cpi.cpi_size, ctx->cpi.n_bins);
 
  LOG(TRACE, "Slow-time FFT...");
  fftw_execute(ctx->slow_time);

  // Fast-time corrections:
  //  -  coherentgain (window gain correction)
  //  -  -3  dB
  //  -  +50 Ohm      (RF)
  //  -  +30          (dB -> dBm)
  //  -  +1/sqrt(N/2) (FFT is unnormalised)
  double fast_correction_db = //window_constants.coherentgain
                            - 3
                            - 10 * log10(50)
                            + 30
                            - 20 * log10(ctx->cpi.chirp_size / 2);

  // Slow-time corrections:
  //  -  coherentgain (window gain correction)
  //  -  1/sqrt(M)    (FFT is unnormalised)
  double slow_correction_db = fast_correction_db
                            //window_constants.coherentgain
                            - 20 * log10(ctx->cpi.cpi_size);

  LOG(TRACE, "Applying corrections...");
  for (size_t i = 0; i < ctx->cpi.fbuffer_size; ++i)
  {
    double re = ctx->cpi.freq_spectrum[i][0];
    double im = ctx->cpi.freq_spectrum[i][1];
    ctx->cpi.power_spectrum_dbm[i]  = 20 * log10(re * re + im * im);
    ctx->cpi.power_spectrum_dbm[i] += fast_correction_db;

    re = ctx->cpi.range_doppler[i][0];
    im = ctx->cpi.range_doppler[i][1];
    ctx->cpi.range_doppler_dbm[i]  = 20 * log10(re * re + im * im);
    ctx->cpi.range_doppler_dbm[i] += slow_correction_db;
  }

  dt = elapsed_milliseconds() - dt;
  LOG_FMT(TRACE, "Processing took %.2f ms", dt);
}
