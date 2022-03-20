#include "daq.h"
#include "fmcw.h"
#include "util.h"
#include "window.h"
#include "ui.h"
#include "thread.h"
#include "main.h"

#include <stdio.h>
#include <time.h>

// For debugging
#include <userint.h>
#include "outputpanel.h"

void main_thread_routine(thread_t *, void *);

int main(int argc, char *argv[])
{
  timer_init();

  daq_t *daq = daq_init(0x14, 0);
  options_t options =
  {
    .proc = {
      .chirp_size = 2048, .cpi_size = 64, .frame_size = 1,
      .window_type = NO_WINDOW
    },
    .dds = {.chirp_duration = 153, .chirp_start = 398.75, .chirp_end = 401.25},
    .log = {.path = ""},
    .daq = {.channel = 0}
  };

  main_thread_arg_t main_thread_arg;
  main_thread_arg.daq = daq;
  main_thread_arg.options = &options;

  thread_t *main_thread = thread_init(&main_thread_routine, &main_thread_arg);

  ui_init(main_thread);
  int status = ui_main_loop();

  thread_stop(main_thread);
  thread_wait_until_idle(main_thread);
  thread_destroy(main_thread);

  daq_destroy(daq);

  return status;
}

// The main acquisition/processing routine.
void main_thread_routine(thread_t *this_thread, void *th_arg)
{
  main_thread_arg_t *arg = th_arg;
  daq_t *daq             = arg->daq;
  options_t *options     = arg->options;

  const uint16_t channel   = options->daq.channel;
  const size_t frame_size  = options->proc.frame_size;
  const size_t buffer_size = options->proc.chirp_size
                           * options->proc.cpi_size * frame_size;

  fmcw_context_t ctx;
  fmcw_context_init(
    &ctx,
    options->proc.chirp_size,
    options->proc.cpi_size,
    options->proc.window_type
  );

  fmcw_cpi_t frame;
  fmcw_cpi_init(&frame, options->proc.chirp_size, options->proc.cpi_size);

  uint16_t *adc_buffer = aligned_malloc(buffer_size * sizeof(uint16_t) * 2);
  uint16_t *buffers[] = {&adc_buffer[0], &adc_buffer[buffer_size]};

  time_t t = time(NULL);
  struct tm *tm_info = localtime(&t);

  char out_file_name[32];
  strftime(out_file_name, SIZEOF_ARRAY(out_file_name),
           "%Y-%m-%d_%H-%M-%S_raw.csv", tm_info);

  //FILE *out_file = fopen(out_file_name, "w");
  FILE *out_file = NULL;

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

    // Check if we should stop acquiring data
    quit = thread_should_stop(this_thread);

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
    memset(frame.power_spectrum_dbm, 0, frame.fbuffer_size * sizeof(double));
    memset(frame.range_doppler_dbm,  0, frame.fbuffer_size * sizeof(double));

    for (size_t j = 0; j < frame_size; ++j)
    {
      for (size_t k = 0; k < ctx.cpi.buffer_size; ++k)
        ctx.cpi.volts[k] = buffers[buf_idx][k] - 32768.;

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

    if (out_file)
    {
      char s[16];
      for (size_t j = 0; j < frame.cpi_size; ++j)
      {
        for (size_t k = 0; k < frame.n_bins; ++k)
        {
          size_t l = j * frame.n_bins + k;
          int len = sprintf(s, "%.2f,", frame.range_doppler_dbm[l]);
          fwrite(s, sizeof(char), len, out_file);
        }

        fputc('\n', out_file);
      }
    }

    // Plot current results
    ui_clear_plots();
    ui_plot_frame(&frame);
    
    PlotY(ui_handles.out_panel,
         PANEL_OUT_RANGE_TIME,
         ctx.cpi.volts,
         ctx.cpi.chirp_size,
         VAL_DOUBLE,
         VAL_THIN_LINE, VAL_NO_POINT,
         VAL_SOLID, 1, VAL_GREEN);

    dt = elapsed_milliseconds() - dt;
    LOG_FMT(TRACE, "Frame processing time: %.2f ms", dt);
  }

  if (out_file)
    fclose(out_file);

  aligned_free(adc_buffer);
  fmcw_cpi_destroy(&frame);
  fmcw_context_destroy(&ctx);
}
