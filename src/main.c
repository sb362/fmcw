#include "fmcw.h"
#include "util.h"

#include <stdio.h>

//#define __MINGW32__
//#include <pthread.h>
//#undef __MINGW32__

int main(int argc, char *argv[])
{
  timer_init();

  fmcw_context_t ctx;
  fmcw_context_init(&ctx, 2048, 64);

  uint16_t *adc_buffer = aligned_malloc(ctx.cpi.buffer_size * sizeof(uint16_t));
  
  size_t read;
  FILE *data = fopen("data/test.dat", "rb");
  
  size_t n = 0;
  double dt = elapsed_milliseconds();
  while ((read = fread(adc_buffer,
                       sizeof(uint16_t),
                       ctx.cpi.buffer_size,
                       data)) > 0)
  {
    for (size_t i = 0; i < ctx.cpi.buffer_size; ++i)
      ctx.cpi.volts[i] = adc_buffer[i];
    fmcw_process(&ctx);

    ++n;
  }
  dt = elapsed_milliseconds() - dt;

  LOG_FMT(INFO, "Processed %zu frames in %.3f ms", n, dt);

  fclose(data);

  aligned_free(adc_buffer);
  fmcw_context_destroy(&ctx);
  return 0;
}
