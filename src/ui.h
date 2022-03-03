#ifndef UI_H
#define UI_H

#include "fmcw.h"

int ui_init(void);
void ui_cleanup(void);

int ui_main_loop(void);

void ui_clear_plots(void);
void ui_plot_frame(const fmcw_context_t *ctx);

#endif // UI_H
