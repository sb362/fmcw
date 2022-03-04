#include "daq.h"
#include "fmcw.h"
#include "util.h"
#include "window.h"
#include "ui.h"

#include "mainthread.h"

#include <stdio.h>

void *main_thread(void *);

int main(int argc, char *argv[])
{
  timer_init();

  daq_t *daq = daq_init(0x14, 0);
  options_t options =
  {
    .proc = {
      .chirp_size = 2048, .cpi_size = 64,
      .window_type = NO_WINDOW, .avg_time = 1.
    },
    .dds = {.chirp_duration = 153, .chirp_start = 398.75, .chirp_end = 401.25},
    .log = {.path = ""}
  };

  start_main_thread(daq, &options);

  int status = ui_main_loop();

  for (int i = 0; i < 1e9; ++i) {};

  stop_main_thread();
  daq_destroy(daq);

  return status;
}
