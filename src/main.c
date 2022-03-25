#include "daq.h"
#include "fmcw.h"
#include "thread.h"
#include "ui.h"
#include "util.h"
#include "window.h"
#include "main.h"

#include <stdio.h>
#include <time.h>

const options_t default_options =
{
  .proc = {
    .chirp_size = 2048, .cpi_size = 64, .frame_size = 1,
    .window_type = NO_WINDOW
  },
  .dds = {.chirp_duration = 153, .chirp_start = 398.75, .chirp_end = 401.25},
  .log = {.path = ""},
  .daq = {.channel = 0, .sampling_rate = 13.3333333, .continuous = 0},
};

daq_t *daq;
options_t options;
pthread_mutex_t options_mutex;

thread_t *main_thread;

// The main acquisition/processing routine.
void main_thread_routine(thread_t *this_thread, void *th_arg)
{
  const size_t frame_size  = options.proc.frame_size;

  size_t doppler_range_bin = options.proc.dopper_range_bin;
  uint16_t channel         = options.daq.channel;

  fmcw_context_t ctx;
  fmcw_context_init(
    &ctx,
    options.proc.chirp_size,
    options.proc.cpi_size,
    options.proc.window_type
  );

  // Averaged range profile
  double *avg_range_profile = aligned_malloc(
    sizeof(double) * ctx.cpi.n_bins
  );
  const size_t fbuffer_bytes = ctx.cpi.fbuffer_size * sizeof(double);

  // Averaged range-Doppler matrix
  double *avg_range_doppler = aligned_malloc(fbuffer_bytes);

  // Doppler moments
  double doppler_moments[4];
  
  uint16_t *adc_buffer = aligned_malloc(
    ctx.cpi.buffer_size * frame_size * sizeof(uint16_t) * 2
  );
  uint16_t *buffers[] = {&adc_buffer[0],
                         &adc_buffer[ctx.cpi.buffer_size * frame_size]};

  // Acquire first frame
  bool quit = daq_acquire(daq,
                          channel,
                          buffers[0],
                          ctx.cpi.chirp_size,
                          ctx.cpi.cpi_size * frame_size,
                          true) <= 0;
  
  for (int buf_idx = 0; !quit; buf_idx ^= 1)
  {
    // Wait for previous transfer
    daq_await(daq);

    // Check if we should immediately stop
    if (thread_should_stop(this_thread))
      break;

    // Check if we should stop acquiring data / options have changed
    pthread_mutex_lock(&options_mutex);
    quit = !options.daq.continuous;

    doppler_range_bin = options.proc.dopper_range_bin;
    channel = options.daq.channel;
    pthread_mutex_unlock(&options_mutex);

    // Request next frame
    if (!quit)
      quit = daq_acquire(daq,
                         channel,
                         buffers[buf_idx ^ 1],
                         ctx.cpi.chirp_size,
                         ctx.cpi.cpi_size * frame_size,
                         true) <= 0;

    // Process current frame
    double dt = elapsed_milliseconds();

    memset(avg_range_profile, 0, sizeof(double) * ctx.cpi.n_bins);
    memset(avg_range_doppler, 0, fbuffer_bytes);

    for (size_t j = 0; j < frame_size; ++j)
    {
      fmcw_copy_volts(&ctx, &buffers[buf_idx][j * ctx.cpi.buffer_size]);
      fmcw_process(&ctx);

      for (size_t k = 0; k < ctx.cpi.cpi_size; ++k)
      {
        for (size_t l = 0; l < ctx.cpi.n_bins; ++l)
        {
          size_t m = k * ctx.cpi.n_bins + l;
          avg_range_profile[l] += ctx.cpi.power_spectrum_dbm[m];
          avg_range_doppler[m] += ctx.cpi.range_doppler_dbm[m];
        }
      }
    }
    
    for (size_t j = 0; j < ctx.cpi.n_bins; ++j)
      avg_range_profile[j] /= ctx.cpi.cpi_size * frame_size;
    
    for (size_t j = 0; j < ctx.cpi.fbuffer_size; ++j)
      avg_range_doppler[j] /= frame_size;

    // Calculate Doppler moments
    const size_t doppler_idx = doppler_range_bin * ctx.cpi.cpi_size;
    double *doppler_spectrum = &avg_range_doppler[doppler_idx];
    fmcw_doppler_moments(doppler_spectrum, ctx.cpi.cpi_size,
                         doppler_moments, SIZEOF_ARRAY(doppler_moments));

    // Plot current results
    ui_clear_plots();
    ui_plot_frame(&ctx, avg_range_profile, avg_range_doppler);
    ui_plot_doppler_spect(doppler_spectrum, ctx.cpi.cpi_size,
                          doppler_moments, SIZEOF_ARRAY(doppler_moments));

    dt = elapsed_milliseconds() - dt;
    LOG_FMT(LVL_TRACE, "Frame processing time: %.2f ms", dt);
  }

  aligned_free(avg_range_profile);
  aligned_free(avg_range_doppler);
  aligned_free(adc_buffer);
  fmcw_context_destroy(&ctx);
}

int main(int argc, char *argv[])
{
  timer_init();

  daq = daq_init(0x14, 0);
  if (daq == NULL)
    return -1;

  options = default_options;
  main_thread = thread_init(&main_thread_routine, NULL);
  pthread_mutex_init(&options_mutex, NULL);

  int status = ui_init();
  if (status == 0)
    status = ui_main_loop();

  thread_stop(main_thread);
  thread_wait_until_idle(main_thread);
  thread_destroy(main_thread);

  daq_destroy(daq);

  pthread_mutex_destroy(&options_mutex);

  return status;
}

void print_options(options_t opts)
{
  printf(
    "FMCW Processing options:\n"
    "  chirp size  = %zu\n"
    "  CPI   size  = %zu\n"
    "  frame size  = %zu\n"
    "  window type = %d\n"
    "  Doppler range bin = %zu\n",
    opts.proc.chirp_size, opts.proc.cpi_size,
    opts.proc.frame_size, opts.proc.window_type,
    opts.proc.dopper_range_bin
  );
  printf(
    "DDS options:\n"
    "  chirp start  = %.2f\n"
    "  chirp end    = %.2f\n"
    "  chirp time   = %.2f\n",
    opts.dds.chirp_start, opts.dds.chirp_end,
    opts.dds.chirp_duration
  );
  printf(
    "DAQ options:\n"
    "  channel       = %d\n"
    "  continuous    = %d\n"
    "  sampling rate = %.2f\n",
    opts.daq.channel, opts.daq.continuous,
    opts.daq.sampling_rate
  );
}
