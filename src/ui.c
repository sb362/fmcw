#include "ui.h"
#include "util.h"
#include "main.h"

#include <userint.h>

#include "controlpanel.h"
#include "outputpanel.h"

ui_handles_t ui_handles;

ColorMapEntry intensity_colour_map[2];

int ui_init()
{
  LOG(DEBUG, "Loading user interface files...");

  ui_handles.ctrl_panel = LoadPanel(0, "controlpanel.uir", PANEL_CTRL);
  if (ui_handles.ctrl_panel <= 0)
  {
    LOG_FMT(ERROR,
            "Failed to load control panel. Error code: %d",
            ui_handles.ctrl_panel);
    return -1;
  }

  ui_handles.out_panel = LoadPanel(0, "outputpanel.uir", PANEL_OUT);
  if (ui_handles.out_panel <= 0)
  {
    LOG_FMT(ERROR,
            "Failed to load output panel. Error code: %d",
            ui_handles.out_panel);
    return -1;
  }

  LOG_FMT(DEBUG, "UI handles: ctrl = %d, out = %d",
          ui_handles.ctrl_panel, ui_handles.out_panel);

  intensity_colour_map[0].color = 0x000000;
  intensity_colour_map[0].dataValue.valDouble = -141;
  intensity_colour_map[1].color = 0xffffff;
  intensity_colour_map[1].dataValue.valDouble = -40;
  
  DisplayPanel(ui_handles.ctrl_panel);
  DisplayPanel(ui_handles.out_panel);

  return 0;
}

void ui_cleanup()
{
  LOG(DEBUG, "Discarding user interface panels...");
  DiscardPanel(ui_handles.ctrl_panel);
  DiscardPanel(ui_handles.out_panel);
}


int ui_main_loop()
{
  LOG(DEBUG, "Entering UI main loop...");
  return RunUserInterface();
}

void ui_clear_plots()
{
  DeleteGraphPlot(ui_handles.out_panel,
                  PANEL_OUT_POWER_SPECT,
                  -1, VAL_DELAYED_DRAW);
  
  DeleteGraphPlot(ui_handles.out_panel,
                  PANEL_OUT_RANGE_DOPPLER,
                  -1, VAL_DELAYED_DRAW);
}

void ui_plot_frame(const fmcw_context_t *ctx)
{
  PlotXY(ui_handles.out_panel,
         PANEL_OUT_POWER_SPECT,
         ctx->cpi.range,
         ctx->cpi.power_spectrum_dbm,
         ctx->cpi.n_bins,
         VAL_DOUBLE, VAL_DOUBLE,
         VAL_FAT_LINE, VAL_NO_POINT,
         VAL_SOLID, 1, VAL_GREEN);
  
  PlotScaledIntensity(ui_handles.out_panel,
                      PANEL_OUT_RANGE_DOPPLER,
                      ctx->cpi.range_doppler_dbm,
                      ctx->cpi.cpi_size, ctx->cpi.n_bins,
                      VAL_DOUBLE, 1, 0, 1, 0,
                      intensity_colour_map,
                      VAL_BLACK,
                      SIZEOF_ARRAY(intensity_colour_map),
                      1, 0);

  double min = 999999, max = -999999, avg = 0;
  for (int i = 0; i < ctx->cpi.fbuffer_size; ++i)
  {
    double v = ctx->cpi.range_doppler_dbm[i];
    avg += v;
    if (v < min)
        min = v;
    if (v > max)
        max = v;
  }
  
  avg /= ctx->cpi.fbuffer_size;
  printf("min = %.3f, max = %.3f, avg = %.3f\n", min, max, avg);
}

int CVICALLBACK ui_quit_event(int panel, int control, int event, void *arg,
                              int event_arg1, int event_arg2)
{
  if (event == EVENT_COMMIT)
    QuitUserInterface(0);

  return 0;
}
