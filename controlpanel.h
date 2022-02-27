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
#define  PANEL_CTRL_DAQ_SWITCH            2       /* control type: binary, callback function: (none) */
#define  PANEL_CTRL_DAQ_LED               3       /* control type: LED, callback function: (none) */
#define  PANEL_CTRL_TX_SWITCH             4       /* control type: binary, callback function: (none) */
#define  PANEL_CTRL_TX_LED                5       /* control type: LED, callback function: (none) */
#define  PANEL_CTRL_RANGE_RES             6       /* control type: ring, callback function: (none) */
#define  PANEL_CTRL_CPI_LEN               7       /* control type: numeric, callback function: (none) */


     /* Control Arrays: */

#define  DAQ                              1
#define  TX                               2

     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* (no callbacks specified in the resource file) */ 


#ifdef __cplusplus
    }
#endif