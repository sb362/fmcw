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

#define  PANEL_CTRL                       1
#define  PANEL_CTRL_TX_SWITCH             2       /* control type: binary, callback function: ui_event */
#define  PANEL_CTRL_TX_LED                3       /* control type: LED, callback function: (none) */
#define  PANEL_CTRL_RANGE_RES             4       /* control type: numeric, callback function: ui_event */
#define  PANEL_CTRL_CPI_LEN               5       /* control type: numeric, callback function: ui_event */
#define  PANEL_CTRL_BANDWIDTH             6       /* control type: numeric, callback function: ui_event */
#define  PANEL_CTRL_AVG_COUNT             7       /* control type: numeric, callback function: ui_event */
#define  PANEL_CTRL_AVG_TIME              8       /* control type: numeric, callback function: ui_event */
#define  PANEL_CTRL_CHIRP_DURATION        9       /* control type: numeric, callback function: ui_event */
#define  PANEL_CTRL_CENTRE_FREQ           10      /* control type: numeric, callback function: ui_event */
#define  PANEL_CTRL_VELOCITY_RES          11      /* control type: numeric, callback function: ui_event */
#define  PANEL_CTRL_TEXT_RANGE            12      /* control type: textMsg, callback function: (none) */
#define  PANEL_CTRL_TEXT_VELOCITY         13      /* control type: textMsg, callback function: (none) */
#define  PANEL_CTRL_DAQ_CHANNEL           14      /* control type: numeric, callback function: ui_event */
#define  PANEL_CTRL_BUTTON_QUIT           15      /* control type: command, callback function: ui_quit_event */
#define  PANEL_CTRL_TEXT_AVERAGE          16      /* control type: textMsg, callback function: (none) */


     /* Control Arrays: */

#define  DAQ                              1
#define  TX                               2

     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK ui_event(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ui_quit_event(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif