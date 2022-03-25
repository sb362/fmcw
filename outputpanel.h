/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL_OUT                        1
#define  PANEL_OUT_DAQ_CONT_SWITCH        2       /* control type: binary, callback function: ui_event */
#define  PANEL_OUT_DAQ_LED                3       /* control type: LED, callback function: (none) */
#define  PANEL_OUT_RANGE_TIME             4       /* control type: graph, callback function: (none) */
#define  PANEL_OUT_RANGE_DOPPLER          5       /* control type: graph, callback function: (none) */
#define  PANEL_OUT_DOPPLER_SPECT          6       /* control type: graph, callback function: (none) */
#define  PANEL_OUT_WINDOW_TYPE            7       /* control type: ring, callback function: ui_event */
#define  PANEL_OUT_POWER_SPECT            8       /* control type: graph, callback function: (none) */
#define  PANEL_OUT_DAQ_SCAN               9       /* control type: command, callback function: ui_event */
#define  PANEL_OUT_BUTTON_QUIT            10      /* control type: command, callback function: ui_quit_event */
#define  PANEL_OUT_DOPPLER_RANGE_BIN      11      /* control type: numeric, callback function: ui_event */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK ui_event(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ui_quit_event(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif