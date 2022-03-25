#ifndef UI_H
#define UI_H

#include "fmcw.h"
#include "thread.h"

int ui_init(void);
void ui_cleanup(void);

int ui_main_loop(void);

void ui_clear_plots(void);
void ui_plot_frame(const fmcw_context_t *ctx,
                   const double *range_profile, const double *range_doppler);
void ui_plot_doppler_spect(const double *spectrum, size_t size,
                           const double *moments,  size_t nmoments);

#endif // UI_H
