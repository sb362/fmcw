#include "window.h"
#include "util.h"

#include <math.h>

void init_window_table(win_table_t *tbl, win_type_t type, size_t size)
{
  tbl->size = size;
  tbl->coherent_gain = 0.;

  tbl->factors = aligned_malloc(size * sizeof(double));
  for (size_t i = 0; i < size; ++i)
    tbl->factors[i] = 0.;
}

void destroy_window_table(win_table_t *tbl)
{
  aligned_free(tbl->factors);
  tbl->factors = NULL;
  tbl->size = 0;
  tbl->coherent_gain = 0.;
}
