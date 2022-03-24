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
  const size_t buffer_size = options.proc.chirp_size
                           * options.proc.cpi_size * frame_size;

  size_t doppler_range_bin = options.proc.dopper_range_bin;
  uint16_t channel         = options.daq.channel;

  fmcw_context_t ctx;
  fmcw_context_init(
    &ctx,
    options.proc.chirp_size,
    options.proc.cpi_size,
    options.proc.window_type
  );

  // Contains averaged power spectrum and range-Doppler profiles
  fmcw_cpi_t frame;
  fmcw_cpi_init(&frame, options.proc.chirp_size, options.proc.cpi_size);

  uint16_t *adc_buffer = aligned_malloc(buffer_size * sizeof(uint16_t) * 2);
  uint16_t *buffers[] = {&adc_buffer[0], &adc_buffer[buffer_size]};

  double doppler_moments[4];

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

    fmcw_copy_volts(&ctx, buffers[buf_idx]);
    fmcw_process(&ctx);

    const size_t fbuffer_bytes = frame.fbuffer_size * sizeof(double);
    memcpy(frame.power_spectrum_dbm, ctx.cpi.power_spectrum_dbm, fbuffer_bytes);
    memcpy(frame.range_doppler_dbm,  ctx.cpi.range_doppler_dbm,  fbuffer_bytes);

    for (size_t j = 1; j < frame_size; ++j)
    {
      fmcw_copy_volts(&ctx, buffers[buf_idx]);
      fmcw_process(&ctx);

      for (size_t k = 0; k < frame.fbuffer_size; ++k)
      {
        frame.power_spectrum_dbm[k] += ctx.cpi.power_spectrum_dbm[k];
        frame.range_doppler_dbm [k] += ctx.cpi.range_doppler_dbm [k];
      }
    }

    for (size_t j = 0; j < frame.fbuffer_size; ++j)
    {
      frame.power_spectrum_dbm[j] /= frame_size;
      frame.range_doppler_dbm [j] /= frame_size;
    }

    // Calculate Doppler moments
    const size_t doppler_idx = doppler_range_bin * frame.cpi_size;
    double *doppler_spectrum = &frame.range_doppler_dbm[doppler_idx];
    fmcw_doppler_moments(doppler_spectrum, frame.cpi_size,
                         doppler_moments, SIZEOF_ARRAY(doppler_moments));

    // Plot current results
    ui_clear_plots();
    ui_plot_frame(&frame);
    ui_plot_doppler_spect(doppler_spectrum, frame.cpi_size,
                          doppler_moments, SIZEOF_ARRAY(doppler_moments));

    dt = elapsed_milliseconds() - dt;
    LOG_FMT(LVL_TRACE, "Frame processing time: %.2f ms", dt);
  }

  aligned_free(adc_buffer);
  fmcw_cpi_destroy(&frame);
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
