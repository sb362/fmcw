#include "ui.h"
#include "util.h"

#include <userint.h>

#include "controlpanel.h"
#include "outputpanel.h"

static ColorMapEntry intensity_colour_map[] =
{
  0x000090, 0x000094, 0x000098, 0x00009c, 0x0000a0,
  0x0000a4, 0x0000a8, 0x0000ab, 0x0000af, 0x0000b3,
  0x0000b7, 0x0000bb, 0x0000bf, 0x0000c3, 0x0000c7,
  0x0000cb, 0x0000cf, 0x0000d3, 0x0000d7, 0x0000db,
  0x0000df, 0x0000e3, 0x0000e7, 0x0000ea, 0x0000ee,
  0x0000f2, 0x0000f6, 0x0000fa, 0x0000fe, 0x0003ff,
  0x0007ff, 0x000bff, 0x000fff, 0x0013ff, 0x0017ff,
  0x001bff, 0x001fff, 0x0023ff, 0x0027ff, 0x002aff,
  0x002eff, 0x0032ff, 0x0036ff, 0x003aff, 0x003eff,
  0x0042ff, 0x0046ff, 0x004aff, 0x004eff, 0x0052ff,
  0x0056ff, 0x005aff, 0x005eff, 0x0062ff, 0x0066ff,
  0x0069ff, 0x006dff, 0x0071ff, 0x0075ff, 0x0079ff,
  0x007dff, 0x0081ff, 0x0085ff, 0x0089ff, 0x008dff,
  0x0091ff, 0x0095ff, 0x0099ff, 0x009dff, 0x00a1ff,
  0x00a5ff, 0x00a8ff, 0x00acff, 0x00b0ff, 0x00b4ff,
  0x00b8ff, 0x00bcff, 0x00c0ff, 0x00c4ff, 0x00c8ff,
  0x00ccff, 0x00d0ff, 0x00d4ff, 0x00d8ff, 0x00dcff,
  0x00e0ff, 0x00e4ff, 0x00e7ff, 0x00ebff, 0x00efff,
  0x00f3ff, 0x00f7ff, 0x00fbff, 0x00ffff, 0x04fffc,
  0x08fff8, 0x0cfff4, 0x10fff0, 0x14ffec, 0x18ffe8,
  0x1cffe4, 0x20ffe0, 0x24ffdc, 0x27ffd8, 0x2bffd5,
  0x2fffd1, 0x33ffcd, 0x37ffc9, 0x3bffc5, 0x3fffc1,
  0x43ffbd, 0x47ffb9, 0x4bffb5, 0x4fffb1, 0x53ffad,
  0x57ffa9, 0x5bffa5, 0x5fffa1, 0x63ff9d, 0x66ff99,
  0x6aff96, 0x6eff92, 0x72ff8e, 0x76ff8a, 0x7aff86,
  0x7eff82, 0x82ff7e, 0x86ff7a, 0x8aff76, 0x8eff72,
  0x92ff6e, 0x96ff6a, 0x9aff66, 0x9eff62, 0xa2ff5e,
  0xa5ff5a, 0xa9ff57, 0xadff53, 0xb1ff4f, 0xb5ff4b,
  0xb9ff47, 0xbdff43, 0xc1ff3f, 0xc5ff3b, 0xc9ff37,
  0xcdff33, 0xd1ff2f, 0xd5ff2b, 0xd9ff27, 0xddff23,
  0xe1ff1f, 0xe4ff1b, 0xe8ff18, 0xecff14, 0xf0ff10,
  0xf4ff0c, 0xf8ff08, 0xfcff04, 0xffff00, 0xfffb00,
  0xfff700, 0xfff300, 0xffef00, 0xffeb00, 0xffe700,
  0xffe300, 0xffdf00, 0xffdc00, 0xffd800, 0xffd400,
  0xffd000, 0xffcc00, 0xffc800, 0xffc400, 0xffc000,
  0xffbc00, 0xffb800, 0xffb400, 0xffb000, 0xffac00,
  0xffa800, 0xffa400, 0xffa000, 0xff9c00, 0xff9900,
  0xff9500, 0xff9100, 0xff8d00, 0xff8900, 0xff8500,
  0xff8100, 0xff7d00, 0xff7900, 0xff7500, 0xff7100,
  0xff6d00, 0xff6900, 0xff6500, 0xff6100, 0xff5d00,
  0xff5a00, 0xff5600, 0xff5200, 0xff4e00, 0xff4a00,
  0xff4600, 0xff4200, 0xff3e00, 0xff3a00, 0xff3600,
  0xff3200, 0xff2e00, 0xff2a00, 0xff2600, 0xff2200,
  0xff1e00, 0xff1b00, 0xff1700, 0xff1300, 0xff0f00,
  0xff0b00, 0xff0700, 0xff0300, 0xfe0000, 0xfa0000,
  0xf60000, 0xf20000, 0xee0000, 0xea0000, 0xe60000,
  0xe20000, 0xde0000, 0xdb0000, 0xd70000, 0xd30000,
  0xcf0000, 0xcb0000, 0xc70000, 0xc30000, 0xbf0000,
  0xbb0000, 0xb70000, 0xb30000, 0xaf0000, 0xab0000,
  0xa70000, 0xa30000, 0x9f0000, 0x9c0000, 0x980000,
  0x940000, 0x900000, 0x8c0000, 0x880000, 0x840000,
  0x800000
};

static struct
{
  int out_panel, ctrl_panel;
} ui_handles;

int ui_init()
{
  LOG(DEBUG, "Loading user interface files...");

  ui_handles.ctrl_panel = LoadPanel(0, "controlpanel.uir", PANEL_CTRL);
  if (ui_handles.ctrl_panel <= 0)
  {
    LOG_FMT(FATAL,
            "Failed to load control panel. Error code: %d",
            ui_handles.ctrl_panel);
    return -1;
  }

  ui_handles.out_panel = LoadPanel(0, "outputpanel.uir", PANEL_OUT);
  if (ui_handles.out_panel <= 0)
  {
    LOG_FMT(FATAL,
            "Failed to load output panel. Error code: %d",
            ui_handles.out_panel);
    return -1;
  }

  DisplayPanel(ui_handles.ctrl_panel);
  DisplayPanel(ui_handles.out_panel);
}


void ui_cleanup()
{
  DiscardPanel(ui_handles.ctrl_panel);
  DiscardPanel(ui_handles.out_panel);
}


int ui_main_loop()
{
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
}

int CVICALLBACK ui_value_changed(int panel, int control, int event, void *arg,
                                 int event_arg1, int event_arg2)
{
  switch (control)
  {
  case PANEL_CTRL_DAQ_SWITCH:
    break;
  case PANEL_CTRL_TX_SWITCH:
    break;
  // Range
  case PANEL_CTRL_BANDWIDTH:
    break;
  case PANEL_CTRL_RANGE_RES:
    break;
  // Velocity
  case PANEL_CTRL_CPI_LEN:
    break;
  case PANEL_CTRL_VELOCITY_RES:
    break;
  // Averaging
  case PANEL_CTRL_AVG_COUNT:
    break;
  case PANEL_CTRL_AVG_TIME:
    break;
  }

  return 0;
}