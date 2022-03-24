#include "ui.h"
#include "util.h"
#include "main.h"

#include <userint.h>

#include "controlpanel.h"
#include "outputpanel.h"

typedef struct
{
  int out_panel, ctrl_panel;
} ui_handles_t;
ui_handles_t ui_handles;

ColorMapEntry intensity_colour_map[2];

extern daq_t *daq;
extern options_t options;
extern pthread_mutex_t options_mutex;

extern thread_t *main_thread;

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

  LOG_FMT(DEBUG, "panel = %d, control = %d, event = %d, arg1 = %d, arg2 = %d",
          panel, control, event, event_arg1, event_arg2);

  options_t new_options = options;

  int is_running = !thread_is_idle(main_thread);
  int needs_restart = 0;
  int start = 0, continuous = new_options.daq.continuous;

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

      unsigned range_res = SPEED_OF_LIGHT / (2 * bandwidth);
      assert(10 <= range_res && range_res <= 30);

      SetCtrlVal(panel, PANEL_CTRL_RANGE_RES, range_res);

      needs_restart = 1;
      break;
    case PANEL_CTRL_RANGE_RES:
      unsigned range_res = 0;
      GetCtrlVal(panel, control, &range_res);

      assert(10 <= range_res && range_res <= 30);

      double bandwidth = SPEED_OF_LIGHT / (2 * range_res);
      SetCtrlVal(panel, PANEL_CTRL_BANDWIDTH, bandwidth);

      needs_restart = 1;
      break;
    // Velocity
    case PANEL_CTRL_CPI_LEN:
      GetCtrlVal(panel, control, &new_options.proc.cpi_size);

      assert(    new_options.proc.cpi_size >= 64
              && new_options.proc.cpi_size <= 256);

      needs_restart = 1;
      break;
    case PANEL_CTRL_VELOCITY_RES:
      double velocity_res;
      GetCtrlVal(panel, control, &velocity_res);
      
      new_options.proc.cpi_size = RADAR_WAVELENGTH
                                / (2 * velocity_res)
                                / (new_options.dds.chirp_duration * 1e-6);

      assert(    new_options.proc.cpi_size >= 64
              && new_options.proc.cpi_size <= 256);
      
      SetCtrlVal(panel, PANEL_CTRL_CPI_LEN, new_options.proc.cpi_size);

      needs_restart = 1;
      break;
    // Averaging
    case PANEL_CTRL_AVG_COUNT:
      GetCtrlVal(panel, control, &new_options.proc.frame_size);

      assert(    new_options.proc.frame_size >= 64
              && new_options.proc.frame_size <= 256);

      double frame_time = new_options.dds.chirp_duration
                        * new_options.proc.cpi_size * 2
                        * new_options.proc.frame_size;

      needs_restart = 1;
      break;
    case PANEL_CTRL_AVG_TIME:
      double avg_time;
      GetCtrlVal(panel, control, &avg_time);

      new_options.proc.frame_size = avg_time
                                  / (new_options.dds.chirp_duration / 1000)
                                  / (new_options.proc.cpi_size * 2);
      SetCtrlVal(panel, PANEL_CTRL_AVG_COUNT, new_options.proc.frame_size);

      needs_restart = 1;
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
      start = 1;
      break;
    case PANEL_OUT_WINDOW_TYPE:
      int type;
      GetCtrlVal(panel, control, &type);
      options.proc.window_type = (win_type_t)type;

      needs_restart = 1;
      break;
    default:
      break;
    }
  }
  
  if (is_running && needs_restart)
  {
    thread_stop(main_thread);
    thread_wait_until_idle(main_thread);
  }

  options = new_options;

  if (    (is_running && needs_restart && options.daq.continuous)
       || (start && !is_running))
    thread_start(main_thread);

  return 0;
}

int ui_init()
{
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

void ui_plot_frame(const fmcw_cpi_t *frame)
{
  PlotY(ui_handles.out_panel,
         PANEL_OUT_POWER_SPECT,
         frame->power_spectrum_dbm,
         frame->n_bins,
         VAL_DOUBLE,
         VAL_THIN_LINE, VAL_NO_POINT,
         VAL_SOLID, 1, VAL_GREEN);
  
  PlotScaledIntensity(ui_handles.out_panel,
                      PANEL_OUT_RANGE_DOPPLER,
                      frame->range_doppler_dbm,
                      frame->cpi_size, frame->n_bins,
                      VAL_DOUBLE, 1, 0, 1, 0,
                      intensity_colour_map,
                      VAL_CYAN,
                      SIZEOF_ARRAY(intensity_colour_map),
                      1, 0);
}

void ui_plot_doppler_spect(const double *spectrum, size_t size,
                           const double *moments, size_t nmoments)
{
  PlotY(ui_handles.out_panel,
          PANEL_OUT_DOPPLER_SPECT,
          spectrum,
          size,
          VAL_DOUBLE,
          VAL_THIN_LINE, VAL_NO_POINT,
          VAL_SOLID, 1, VAL_GREEN);
}
