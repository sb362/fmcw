#include "daq.h"
#include "fmcw.h"
#include "util.h"
#include "window.h"
#include "ui.h"
#include "thread.h"
#include "main.h"

#include <stdio.h>

typedef struct
{
  daq_t *daq;
  options_t *options;
} arg_t;

void main_thread_routine(thread_t *, void *);
thread_t *main_thread;
arg_t main_thread_arg; // todo: pass this to ui_event via arg instead

int main(int argc, char *argv[])
{
  timer_init();

  daq_t *daq = daq_init(0x14, 0);
  options_t options =
  {
    .proc = {
      .chirp_size = 2048, .cpi_size = 64, .frame_size = 1,
      .window_type = NO_WINDOW
    },
    .dds = {.chirp_duration = 153, .chirp_start = 398.75, .chirp_end = 401.25},
    .log = {.path = ""},
    .daq = {.channel = 0}
  };

  main_thread_arg.daq = daq;
  main_thread_arg.options = &options;

  main_thread = thread_init(&main_thread_routine, &main_thread_arg);

  ui_init();
  int status = ui_main_loop();

  thread_stop(main_thread);
  thread_wait_until_idle(main_thread);
  thread_destroy(main_thread);

  daq_destroy(daq);

  return status;
}

// The main acquisition/processing routine.
void main_thread_routine(thread_t *this_thread, void *arg)
{
  daq_t *daq = ((arg_t *)arg)->daq;
  options_t *options = ((arg_t *)arg)->options;

  if (daq_set_channel(daq, options->daq.channel))
  {
    LOG(WARN, "Failed to set DAQ channel");
    return;
  }

  const size_t frame_size  = options->proc.frame_size;
  const size_t buffer_size = options->proc.chirp_size
                           * options->proc.cpi_size * frame_size;

  fmcw_context_t ctx;
  fmcw_context_init(
    &ctx,
    options->proc.chirp_size,
    options->proc.cpi_size,
    options->proc.window_type
  );

  uint16_t *adc_buffer = aligned_malloc(buffer_size * sizeof(uint16_t) * 2);
  uint16_t *buffers[] = {&adc_buffer[0], &adc_buffer[buffer_size]};

  // Acquire first frame
  bool quit = daq_acquire(daq,
                          buffers[0],
                          ctx.cpi.chirp_size,
                          ctx.cpi.cpi_size * frame_size,
                          true) <= 0;
  
  for (int i = 0; !quit; i ^= 1)
  {
    // Wait for previous transfer
    daq_await(daq);

    // Check if we should stop acquiring data
    quit = thread_should_stop(this_thread);

    // Request next frame
    if (!quit)
      quit = daq_acquire(daq,
                         buffers[i ^ 1],
                         ctx.cpi.chirp_size,
                         ctx.cpi.cpi_size * frame_size,
                         true) <= 0;

    // Process current frame
    for (size_t j = 0; j < frame_size; ++j)
    {
      for (size_t k = 0; k < ctx.cpi.buffer_size; ++k)
        ctx.cpi.volts[k] = buffers[i][k] - 32768.;

      fmcw_process(&ctx);
    }

    // Plot current results
    ui_clear_plots();
    ui_plot_frame(&ctx);
  }

  aligned_free(adc_buffer);
  fmcw_context_destroy(&ctx);
}

#ifdef _CVI_
#include <userint.h>

#include "controlpanel.h"
#include "outputpanel.h"

int CVICALLBACK ui_event(int panel, int control, int event, void *arg,
                         int event_arg1, int event_arg2)
{
  if (event != EVENT_COMMIT)
    return 0;

  LOG_FMT(DEBUG, "panel = %d, control = %d, event = %d, arg1 = %d, arg2 = %d",
          panel, control, event, event_arg1, event_arg2);

  options_t options = *main_thread_arg.options;

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

  *main_thread_arg.options = options;

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

#endif // _CVI_
