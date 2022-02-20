#include "util.h"

#include <fftw3.h>

const char *log_level_strs[] = {
  "FATAL", "WARN ", "INFO ", "DEBUG", "TRACE"
};

#ifdef _WIN32
#include <Windows.h>

static double perf_freq = 0.0;
static uint64_t timer_start = 0;

void timer_init()
{
  LARGE_INTEGER li;

  QueryPerformanceFrequency(&li);
  perf_freq = (double)li.QuadPart / 1000;

  QueryPerformanceCounter(&li);
  timer_start = li.QuadPart;
}

double elapsed_milliseconds()
{
  LARGE_INTEGER li;
  QueryPerformanceCounter(&li);

  return (double)(li.QuadPart - timer_start) / perf_freq;
}
#else
void timer_init()
{
  
}

double elapsed_milliseconds()
{
  return 0.0;
}
#endif

void *safe_malloc(size_t n)
{
  LOG_FMT(TRACE, "Allocating %llu bytes", n);
  void *ptr = malloc(n);
  if (ptr == NULL)
  {
    FATAL_ERROR("Out of memory", "Failed to allocate %llu bytes of memory", n);
    exit(1);
  }

  return ptr;
}

void *aligned_malloc(size_t n)
{
  LOG_FMT(TRACE, "Allocating %llu bytes", n);
  void *ptr = fftw_malloc(n);
  if (ptr == NULL)
  {
    FATAL_ERROR("Out of memory", "Failed to allocate %llu bytes of memory", n);
    exit(1);
  }

  return ptr;
}

void aligned_free(void *mem)
{
  fftw_free(mem);
}
