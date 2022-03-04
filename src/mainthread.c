#include "daq.h"
#include "fmcw.h"
#include "util.h"
#include "window.h"
#include "ui.h"
#include "mainthread.h"

#include <assert.h>

typedef struct
{
  daq_t *daq;
	options_t *options;

  bool stop;
  pthread_mutex_t mutex;

	pthread_t th;

  bool running;
} thread_t;

thread_t main_thread;

void *main_thread_routine(void *);

void start_main_thread(daq_t *daq, options_t *options)
{
  assert(daq && options);
  assert(!main_thread.running);

  LOG(DEBUG, "Starting main thread");

  main_thread.daq = daq;
  main_thread.options = options;
  main_thread.stop = false;
  main_thread.running = true;

	pthread_create(&main_thread.th, NULL, main_thread_routine, NULL);
}

void stop_main_thread()
{
  if (!main_thread.running)
    return;

  LOG(DEBUG, "Stopping main thread");

  pthread_mutex_lock(&main_thread.mutex);
  main_thread.stop = true;
  pthread_mutex_unlock(&main_thread.mutex);
	pthread_join(main_thread.th, NULL);

  LOG(DEBUG, "Main thread stopped");
}

// The main acquisition/processing routine.
void *main_thread_routine(void *arg)
{
  thread_t *this_thread = arg;
  options_t *options = &this_thread->options;
  daq_t *daq = this_thread->daq;

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
    pthread_mutex_lock(&this_thread->mutex);
    quit = this_thread->stop;
    pthread_mutex_unlock(&this_thread->mutex);

		// Plot current results
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
