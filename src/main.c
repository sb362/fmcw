#include "daq.h"
#include "fmcw.h"
#include "util.h"
#include "window.h"
#include "ui.h"

#include <stdio.h>

//#define __MINGW32__
#include <pthread.h>
//#undef __MINGW32__

typedef struct
{
  size_t chirp_size, cpi_size;
  win_type_t window_type;

  double avg_time;
} proc_options_t;

typedef struct
{
  double chirp_start, chirp_end, chirp_duration;
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
} options_t;

typedef struct
{
  daq_t *daq;

  options_t *options;

  bool stop;
  pthread_mutex_t mutex;
} thread_arg_t;

void *main_thread(void *);

int main(int argc, char *argv[])
{
  timer_init();

  options_t options =
  {
    .proc = {
      .chirp_size = 2048, .cpi_size = 64,
      .window_type = NO_WINDOW, .avg_time = 1.
    },
    .dds = {.chirp_duration = 153, .chirp_start = 398.75, .chirp_end = 401.25},
    .log = {.path = ""}
  };

  daq_t *daq = daq_init(0x14, 0);
  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);
  
  thread_arg_t arg;
  arg.daq = daq;
  arg.options = &options;
  arg.stop = false;
  pthread_mutex_init(&arg.mutex, NULL);

  pthread_t thread;
  pthread_create(&thread, NULL, main_thread, &arg);
  pthread_join(thread, NULL);
  pthread_mutex_destroy(&arg.mutex);

  return 0;
}

void *main_thread(void *targ)
{
  thread_arg_t *arg = targ;
  options_t *options = arg->options;
  daq_t *daq = arg->daq;

  const size_t buffer_size  = options->proc.chirp_size * options->proc.cpi_size;
  const double cpi_duration = options->dds.chirp_duration
                            * options->proc.cpi_size;

  fmcw_context_t ctx;
  fmcw_context_init(
    &ctx,
    options->proc.chirp_size,
    options->proc.cpi_size,
    options->proc.window_type
  );

  uint16_t *adc_buffer = aligned_malloc(buffer_size * sizeof(uint16_t) * 2);
  uint16_t *buffers[] = {&adc_buffer[0], &adc_buffer[buffer_size]};

  bool quit = false;

  size_t total_frames = 0;
  double total_time = 0.;
  double dt = elapsed_milliseconds();

  // Acquire first CPI
  if (daq_acquire(daq,
                  buffers[0],
                  ctx.cpi.chirp_size,
                  ctx.cpi.cpi_size,
                  true) <= 0)
    quit = true;
  
  for (int i = 0; !quit; i ^= 1)
  {
    // Await last CPI
    daq_await(daq);

    dt = elapsed_milliseconds() - dt;
    LOG_FMT((dt > 1.05 * cpi_duration) ? WARN : TRACE,
            "Frame time: %.2f ms", dt);

    // Request next CPI
    dt = elapsed_milliseconds();
    if (daq_acquire(daq,
                    buffers[i ^ 1],
                    ctx.cpi.chirp_size,
                    ctx.cpi.cpi_size,
                    true) <= 0)
    {
      break;
    }

    // Process CPI
    double dt_p = elapsed_milliseconds();
    for (int j = 0; j < buffer_size; ++j)
      ctx.cpi.volts[j] = buffers[i][j] - 32768.;

    fmcw_process(&ctx);
    dt_p = elapsed_milliseconds() - dt_p;
    LOG_FMT(TRACE, "Processing took %.2f ms", dt_p);

    ++total_frames;
    total_time += dt_p;

    // Check if we should stop acquiring data
    pthread_mutex_lock(&arg->mutex);
    quit = arg->stop;
    pthread_mutex_unlock(&arg->mutex);

    double dt_g = elapsed_milliseconds();
    ui_clear_plots();
    ui_plot_frame(&ctx);
    dt_g = elapsed_milliseconds() - dt_g;
    LOG_FMT(TRACE, "Plotting took %.2f ms", dt_g);
  }

  if (total_frames)
    LOG_FMT(INFO,
            "Processed %zu CPIs in %.2f ms, avg. time: %.2f ms",
            total_frames, total_time,
            total_time / total_frames);

  aligned_free(adc_buffer);
  fmcw_context_destroy(&ctx);

  return NULL;
}
