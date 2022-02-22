#include "window.h"
#include "util.h"

#include <math.h>

// Formula taken from
// https://zone.ni.com/reference/en-XX/help/370051AG-01/cvi/libref/cviblkharriswin/
double blackman_harris(int n, int N)
{
  double k = (M_2PI * n) / N;
  return    0.42323
          - 0.49755 * cos(    k)
          + 0.07922 * cos(2 * k);
}

// Formula taken from
// https://zone.ni.com/reference/en-XX/help/370051AG-01/cvi/libref/cviflattopwin/
double flat_top(int n, int N)
{
  double k = (M_2PI * n) / N;
  return    0.215578948  
          - 0.416631580 * cos(    k)
          + 0.277263158 * cos(2 * k)
          - 0.083578947 * cos(3 * k)
          + 0.006947368 * cos(4 * k);
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
  default:
    LOG_FMT(WARN, "Unknown window type: %d", type);
    return 1.;
  }
}

void win_table_init(win_table_t *tbl, win_type_t type, size_t size)
{
  tbl->size = size;
  tbl->coherent_gain = coherent_gain(type);

  tbl->factors = aligned_malloc(size * sizeof(double));
  for (size_t i = 0; i < size; ++i)
  {
    double f = 1.;
    switch (type)
    {
    case NO_WINDOW:
      break;
    case FLAT_TOP_WINDOW:
      f = flat_top(i, size);
      break;
    case BLACKMAN_HARRIS_WINDOW:
      f = blackman_harris(i, size);
      break;
    default:
      break;
    }

    tbl->factors[i] = f;
  }
}

void win_table_free(win_table_t *tbl)
{
  aligned_free(tbl->factors);
  tbl->factors = NULL;
  tbl->size = 0;
  tbl->coherent_gain = 0.;
}
