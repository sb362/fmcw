#include "fmcw.h"
#include "util.h"
#include "window.h"

#include <math.h>

void fmcw_cpi_init(fmcw_cpi_t *cpi, size_t chirp_size, size_t cpi_size)
{
  LOG_FMT(TRACE, "Initialising CPI (%zu, %zu)", chirp_size, cpi_size);
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

void fmcw_context_init(fmcw_context_t *ctx, size_t chirp_size,
                       size_t cpi_size, win_type_t win_type)
{
  fmcw_cpi_init(&ctx->cpi, chirp_size, cpi_size);

  // Details on fftw_plan_many_dft() are available at:
  // https://www.fftw.org/fftw3_doc/Advanced-Complex-DFTs.html

  LOG(TRACE, "Initialising fast-time FFT plan...");
  int nfast[] = {chirp_size};
  ctx->fast_time = fftw_plan_many_dft_r2c(
    1,        // rank, we are performing 1D transforms along each row
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
    1,               // rank, we are performing 1D transforms along each column
    nslow,           // length of 1D transforms = number of rows (chirps)
    ctx->cpi.n_bins, // number of 1D transforms = number of columns (range bins)
    ctx->cpi.freq_spectrum, // input array
    NULL,                   // unused, for FFT'ing subarrays
    ctx->cpi.n_bins,        // distance between elements in each input column
    1,                      // distance between each transform input (column)
    ctx->cpi.range_doppler, // output array
    NULL,                   // unused, for FFT'ing subarrays
    ctx->cpi.n_bins,        // distance between elements in each output column
    1,                      // distance between each transform output (column)
    FFTW_FORWARD,					  // forward transform
    FFTW_MEASURE            // optimise plan by profiling several FFTs
  );

  LOG(TRACE, "Initialising window tables...");
  win_table_init(&ctx->fast_win, win_type, ctx->cpi.chirp_size);
  win_table_init(&ctx->slow_win, win_type, ctx->cpi.cpi_size);
}

void fmcw_context_destroy(fmcw_context_t *ctx)
{
  fftw_destroy_plan(ctx->fast_time);
  fftw_destroy_plan(ctx->slow_time);
  fmcw_cpi_destroy(&ctx->cpi);
  win_table_free(&ctx->fast_win);
  win_table_free(&ctx->slow_win);
}

void fmcw_process(fmcw_context_t *ctx)
{
  // Fast-time windowing
  for (size_t i = 0; i < ctx->cpi.cpi_size; ++i)
  {
    double *chirp = &ctx->cpi.volts[i * ctx->cpi.chirp_size];
    for (size_t j = 0; j < ctx->cpi.chirp_size; ++j)
    {
      chirp[j] *= ctx->fast_win.factors[j];
    }
  }
  
  // Fast-time FFT
  fftw_execute(ctx->fast_time);

  // Fast-time corrections:
  //  -  +30           (dB -> dBm)
  //  -  coherent gain (window gain correction)
  //  -  -3  dB        (peak-peak to RMS)
  //  -  +50 Ohm       (RF)
  //  -  +1/sqrt(N/2)  (FFT is unnormalised)
  double fast_correction_db = 30
                            - 20 * log10(ctx->fast_win.coherent_gain)
                            - 3
                            - 10 * log10(50)
                            - 20 * log10(ctx->cpi.n_bins)
                            - 20 * log10(32768);

  for (size_t i = 0; i < ctx->cpi.fbuffer_size; ++i)
  {
    double re = ctx->cpi.freq_spectrum[i][0];
    double im = ctx->cpi.freq_spectrum[i][1];
    double m2 = re * re + im * im;

    if (m2 > 0)
      ctx->cpi.power_spectrum_dbm[i] = 10 * log10(m2) + fast_correction_db;
    else
      ctx->cpi.power_spectrum_dbm[i] = -123;
  }

  // Slow-time windowing
  for (size_t i = 0; i < ctx->cpi.cpi_size; ++i)
  {
    double factor = ctx->slow_win.factors[i];
    for (size_t j = 0; j < ctx->cpi.n_bins; ++j)
    {
      size_t k = i * ctx->cpi.n_bins + j;
      ctx->cpi.freq_spectrum[k][0] *= factor;
      ctx->cpi.freq_spectrum[k][1] *= factor;
    }
  }
 
  // Slow-time FFT
  fftw_execute(ctx->slow_time);

  // Slow-time corrections + transpose:
  //  -  coherent gain (window gain correction)
  //  -  1/sqrt(M)     (FFT is unnormalised)
  double slow_correction_db = fast_correction_db
                            - 20 * log10(ctx->slow_win.coherent_gain)
                            - 20 * log10(ctx->cpi.cpi_size);

  for (size_t i = 0; i < ctx->cpi.cpi_size; ++i)
  {
    for (size_t j = 0; j < ctx->cpi.n_bins; ++j)
    {
      size_t k = i * ctx->cpi.n_bins + j;

      double re = ctx->cpi.range_doppler[i][0];
      double im = ctx->cpi.range_doppler[i][1];
      double m2 = re * re + im * im;

      // Transposed index
      size_t l = j * ctx->cpi.cpi_size + i;

      // Frequency shift
      if (i < ctx->cpi.cpi_size / 2)
        l += ctx->cpi.cpi_size / 2;
      else
        l -= ctx->cpi.cpi_size / 2;

      if (m2 > 0)
        ctx->cpi.range_doppler_dbm[l] = 10 * log10(m2) + slow_correction_db;
      else
        ctx->cpi.range_doppler_dbm[l] = -123;
    }
  }
}

void fmcw_copy_volts(fmcw_context_t *ctx, uint16_t *volts)
{
  for (size_t i = 0; i < ctx->cpi.buffer_size; ++i)
    ctx->cpi.volts[i] = volts[i] - 32768.;
}


void fmcw_doppler_moments(double *spectrum, size_t spectrum_size,
                          double *moments, size_t moments_size)
{
  double power[256]; // todo, make MAX_CPI_SIZE or something
  double total = 0;

  moments[0] = 0; // mean
  moments[1] = 0; // variance
  moments[2] = 0; // skew
  moments[3] = 0; // kurtosis

  // First, convert back to linear
  for (size_t i = 0; i < spectrum_size; ++i)
  {
    power[i] = pow(10, spectrum[i] / 10);
    total += power[i];
  }

  for (size_t i = 0; i < spectrum_size; ++i)
  {
    power[i] /= total;
    moments[0] += power[i] * i;
  }
  moments[0] /= total;

  for (size_t i = 0; i < spectrum_size; ++i)
  {
    moments[1] += power[i] * (i - moments[0]) * (i - moments[0]);
  }
  moments[1] /= total;

  double std_dev = sqrt(moments[1]);

  for (size_t n = 2; n < moments_size; ++n)
  {
    for (size_t i = 0; i < spectrum_size; ++i)
      moments[n] += power[i] * pow((i - moments[0]) / std_dev, n + 1);

    moments[n] /= total;
  }
}
