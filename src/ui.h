#ifndef UI_H
#define UI_H

#include "fmcw.h"
#include "thread.h"

typedef struct
{
  int out_panel, ctrl_panel;
} ui_handles_t;
extern ui_handles_t ui_handles;

int ui_init(thread_t *main_thread);
void ui_cleanup(void);

int ui_main_loop(void);

void ui_clear_plots(void);
void ui_plot_frame(const fmcw_cpi_t *cpi);

#endif // UI_H
