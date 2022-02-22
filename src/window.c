#include "window.h"
#include "util.h"

#include <math.h>

void win_table_init(win_table_t *tbl, win_type_t type, size_t size)
{
  tbl->size = size;
  tbl->coherent_gain = 0.;

  tbl->factors = aligned_malloc(size * sizeof(double));
  for (size_t i = 0; i < size; ++i)
    tbl->factors[i] = 0.;
}

void win_table_free(win_table_t *tbl)
{
  aligned_free(tbl->factors);
  tbl->factors = NULL;
  tbl->size = 0;
  tbl->coherent_gain = 0.;
}
