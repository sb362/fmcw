#include "ui.h"
#include "util.h"
#include "main.h"

#include <userint.h>

#include "controlpanel.h"
#include "outputpanel.h"

ui_handles_t ui_handles;
ColorMapEntry intensity_colour_map[2];

// This is horrible
thread_t *main_thread;
main_thread_arg_t *main_thread_arg;

int CVICALLBACK ui_quit_event(int panel, int control, int event, void *arg,
                              int event_arg1, int event_arg2)
{
  if (event == EVENT_COMMIT)
    QuitUserInterface(0);

  return 0;
}

int CVICALLBACK ui_event(int panel, int control, int event, void *arg,
                         int event_arg1, int event_arg2)
{
  if (event != EVENT_COMMIT)
    return 0;

  options_t options = *main_thread_arg->options;

  LOG_FMT(DEBUG, "panel = %d, control = %d, event = %d, arg1 = %d, arg2 = %d",
          panel, control, event, event_arg1, event_arg2);

  bool is_running = !thread_is_idle(main_thread);
  int start_scan = is_running, continuous = 0;
  GetCtrlVal(panel, PANEL_OUT_DAQ_CONT_SWITCH, &continuous);

  if (panel == ui_handles.ctrl_panel)
  {
    switch (control)
    {
    case PANEL_CTRL_TX_SWITCH:
      int enable;
      GetCtrlVal(panel, control, &enable);
      SetCtrlVal(panel, PANEL_CTRL_TX_LED, enable);
      break;
    case PANEL_CTRL_DAQ_CHANNEL:
      GetCtrlVal(panel, control, &options.daq.channel);
      break;
    // Range
    case PANEL_CTRL_BANDWIDTH:
      double bandwidth;
      GetCtrlVal(panel, control, &bandwidth);

      break;
    case PANEL_CTRL_RANGE_RES:
      break;
    // Velocity
    case PANEL_CTRL_CPI_LEN:
      GetCtrlVal(panel, control, &options.proc.cpi_size);

      break;
    case PANEL_CTRL_VELOCITY_RES:
      break;
    // Averaging
    case PANEL_CTRL_AVG_COUNT:
      GetCtrlVal(panel, control, &options.proc.frame_size);

      break;
    case PANEL_CTRL_AVG_TIME:
      break;
    default:
      break;
    }
  }
  else if (panel == ui_handles.out_panel)
  {
    switch (control)
    {
    case PANEL_OUT_DAQ_CONT_SWITCH:
      SetCtrlVal(panel, PANEL_OUT_DAQ_LED, continuous);
      break;
    case PANEL_OUT_DAQ_SCAN:
      start_scan = 1;
      break;
    case PANEL_OUT_WINDOW_TYPE:
      int type;
      GetCtrlVal(panel, control, &type);
      options.proc.window_type = (win_type_t)type;
      break;
    default:
      break;
    }
  }

  LOG_FMT(DEBUG, "start_scan = %d, continuous = %d, is_running = %d", start_scan, continuous, is_running);
  
  if (is_running)
  {
    thread_stop(main_thread);
    thread_wait_until_idle(main_thread);
  }

  *main_thread_arg->options = options;

  if (start_scan)
  {
    thread_start(main_thread);
    
    // Immediately send stop signal.
    // main thread should exit after first iteration
    if (!continuous)
      thread_stop(main_thread);
  }

  return 0;
}

int ui_init(thread_t *th)
{
  main_thread = th;
  main_thread_arg = thread_get_task_arg(main_thread);

  LOG(DEBUG, "Loading user interface files...");

  // Load control panel
  ui_handles.ctrl_panel = LoadPanel(0, "controlpanel.uir", PANEL_CTRL);
  if (ui_handles.ctrl_panel <= 0)
  {
    LOG_FMT(ERROR,
            "Failed to load control panel. Error code: %d",
            ui_handles.ctrl_panel);
    return -1;
  }

  // Load output panel
  ui_handles.out_panel = LoadPanel(0, "outputpanel.uir", PANEL_OUT);
  if (ui_handles.out_panel <= 0)
  {
    LOG_FMT(ERROR,
            "Failed to load output panel. Error code: %d",
            ui_handles.out_panel);
    return -1;
  }

  // Intensity colour map used in range-Doppler plot
  intensity_colour_map[0].color = 0x000000;
  intensity_colour_map[0].dataValue.valDouble = -141;
  intensity_colour_map[1].color = 0xffffff;
  intensity_colour_map[1].dataValue.valDouble = -40;  

  DisplayPanel(ui_handles.ctrl_panel);
  DisplayPanel(ui_handles.out_panel);

  LOG_FMT(DEBUG, "UI handles: ctrl = %d, out = %d",
          ui_handles.ctrl_panel, ui_handles.out_panel);

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

void ui_plot_frame(const fmcw_cpi_t *cpi)
{
  PlotXY(ui_handles.out_panel,
         PANEL_OUT_POWER_SPECT,
         cpi->range,
         cpi->power_spectrum_dbm,
         cpi->n_bins,
         VAL_DOUBLE, VAL_DOUBLE,
         VAL_THIN_LINE, VAL_NO_POINT,
         VAL_SOLID, 1, VAL_GREEN);
  
  PlotScaledIntensity(ui_handles.out_panel,
                      PANEL_OUT_RANGE_DOPPLER,
                      cpi->range_doppler_dbm,
                      cpi->cpi_size, cpi->n_bins,
                      VAL_DOUBLE, 1, 0, 1, 0,
                      intensity_colour_map,
                      VAL_CYAN,
                      SIZEOF_ARRAY(intensity_colour_map),
                      1, 0);
}
