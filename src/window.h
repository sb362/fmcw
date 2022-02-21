#ifndef WINDOW_H
#define WINDOW_H

#include <stdint.h>

typedef enum
{
  NO_WINDOW,
  FLAT_TOP_WINDOW,
  HAN_WINDOW,
  BLACKMAN_WINDOW,
  BLACKMAN_HARRIS_WINDOW
} win_type_t;

typedef struct
{
  double coherent_gain, *factors;
  size_t size;
} win_table_t;

void init_window_table(win_table_t *tbl, win_type_t type, size_t size);
void destroy_window_table(win_table_t *tbl)

#endif // WINDOW_H
