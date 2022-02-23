#include "daq.h"
#include "fmcw.h"
#include "util.h"
#include "window.h"

#include <Windows.h>
#include "Wd-dask64.h"

#include <stdio.h>

//#define __MINGW32__
//#include <pthread.h>
//#undef __MINGW32__

void *acquisition_thread(void *arg);

int main(int argc, char *argv[])
{
  timer_init();

  (void)acquisition_thread(NULL);

  return 0;
}

void *acquisition_thread(void *arg)
{
  const size_t chirp_size = 2048, cpi_size = 64;
  const size_t buffer_size = chirp_size * cpi_size;

  daq_t *daq = daq_init(0x14, 0);

  fmcw_context_t ctx;
  fmcw_context_init(&ctx, chirp_size, cpi_size, BLACKMAN_HARRIS_WINDOW);

  uint16_t *adc_buffer = aligned_malloc(buffer_size * sizeof(uint16_t) * 2);
  uint16_t *buffers[] = {&adc_buffer[0], &adc_buffer[buffer_size]};

  // Acquire first chunk
  double dt_a = elapsed_milliseconds();
  daq_acquire(daq, buffers[0], chirp_size, cpi_size, true);

  double total_time = 0.;
  size_t total_frames = 0.;

  for (int i = 0; ; i ^= 1)
  {
    // Await last chunk
    daq_await(daq);

    dt_a = elapsed_milliseconds() - dt_a;
    LOG_FMT(DEBUG, "Acquisition took %.2f ms", dt_a);
    LOG_FMT(DEBUG, "%d", buffers[i][1234]);

    // Request next chunk
    dt_a = elapsed_milliseconds();
    daq_acquire(daq, buffers[i ^ 1], chirp_size, cpi_size, true);

    // Process first
    double dt_p = elapsed_milliseconds();
    for (int j = 0; j < buffer_size; ++j)
    {
      ctx.cpi.volts[j] = buffers[i][j];
    }
    fmcw_process(&ctx);
    dt_p = elapsed_milliseconds() - dt_p;
    LOG_FMT(DEBUG, "Processing took %.2f ms", dt_p);

    total_time += dt_p;
    total_frames += 1;

    if (total_frames == 1000)
      break;
  }

  printf("%zu samples in %.3f ms, avg: %.3f\n",
         total_frames, total_time,
         total_time / total_frames);

  aligned_free(adc_buffer);
  fmcw_context_destroy(&ctx);

  return NULL;
}
