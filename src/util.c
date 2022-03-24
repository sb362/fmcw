#include "util.h"

#include <fftw3.h>

const char *log_level_strs[] = {
  "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE"
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
#include <time.h>

static double timer_start = 0.0;

void timer_init()
{
  timer_start = elapsed_milliseconds();
}

double elapsed_milliseconds()
{
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);

  return (ts.tv_sec * 1e3 + ts.tv_nsec * 1e-6) - timer_start;
}
#endif

void *safe_malloc(size_t n)
{
  LOG_FMT(LVL_TRACE, "Allocating %zu bytes", n);
  void *ptr = malloc(n);
  if (ptr == NULL)
  {
    LOG_FMT(LVL_ERROR, "Failed to allocate %zu bytes of memory", n);
    exit(1);
  }

  return ptr;
}

void *aligned_malloc(size_t n)
{
  LOG_FMT(LVL_TRACE, "Allocating %zu bytes", n);
  void *ptr = fftw_malloc(n);
  if (ptr == NULL)
  {
    LOG_FMT(LVL_ERROR, "Failed to allocate %zu bytes of memory", n);
    exit(1);
  }

  return ptr;
}

void aligned_free(void *mem)
{
  fftw_free(mem);
}
