#include "window.h"
#include "util.h"

#include <math.h>

// Formula taken from
// https://zone.ni.com/reference/en-XX/help/370051AG-01/cvi/libref/cviblkharriswin/
double blackman_harris(int n, size_t N)
{
  double k = (M_2PI * n) / N;
  return    0.42323
          - 0.49755 * cos(    k)
          + 0.07922 * cos(2 * k);
}

// Formula taken from
// https://zone.ni.com/reference/en-XX/help/370051AG-01/cvi/libref/cviflattopwin/
double flat_top(int n, size_t N)
{
  double k = (M_2PI * n) / N;
  return    0.215578948  
          - 0.416631580 * cos(    k)
          + 0.277263158 * cos(2 * k)
          - 0.083578947 * cos(3 * k)
          + 0.006947368 * cos(4 * k);
}

// Formula taken from
// https://zone.ni.com/reference/en-XX/help/370051AG-01/cvi/libref/cvihanwin/
double han(int n, size_t N)
{
  double k = (M_2PI * n) / N;
  return    0.5  
          - 0.5 * cos(k);
}

// Formula taken from
// https://zone.ni.com/reference/en-XX/help/370051AG-01/cvi/libref/cvibkmanwin/
double blackman(int n, size_t N)
{
  double k = (M_2PI * n) / N;
  return    0.42  
          - 0.50 * cos(    k)
          + 0.08 * cos(2 * k);
}

// Values taken from
// https://zone.ni.com/reference/en-XX/help/370051AG-01/cvi/libref/cviscaledwindowex/
double coherent_gain(win_type_t type)
{
  switch (type)
  {
  case NO_WINDOW:
    return 1.;
  case FLAT_TOP_WINDOW:
    return 0.215578948;
  case BLACKMAN_HARRIS_WINDOW:
    return 0.42323;
  case HAN_WINDOW:
    return 0.5;
  case BLACKMAN_WINDOW:
    return 0.42;
  default:
    LOG_FMT(LVL_WARN, "Unknown window type: %d", type);
    return 1.;
  }
}

double window_func(win_type_t type, int i, size_t size)
{
  switch (type)
  {
    case NO_WINDOW:
      return 1.;
    case FLAT_TOP_WINDOW:
      return flat_top(i, size);
    case BLACKMAN_HARRIS_WINDOW:
      return blackman_harris(i, size);
    case BLACKMAN_WINDOW:
      return blackman(i, size);
    case HAN_WINDOW:
      return han(i, size);
    default:
      return 0.;
    }
}

void win_table_init(win_table_t *tbl, win_type_t type, size_t size)
{
  tbl->size = size;
  tbl->coherent_gain = coherent_gain(type);

  tbl->factors = aligned_malloc(size * sizeof(double));
  for (int i = 0; i < size; ++i)
  {
    tbl->factors[i] = window_func(type, i, size);
  }
}

void win_table_free(win_table_t *tbl)
{
  aligned_free(tbl->factors);
  tbl->factors = NULL;
  tbl->size = 0;
  tbl->coherent_gain = 0.;
}
