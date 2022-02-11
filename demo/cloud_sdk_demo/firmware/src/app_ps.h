/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ps.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_PS_H
#define _APP_PS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "definitions.h"
// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************

#define APP_PS_DBG(level,fmt,...) SYS_DEBUG_PRINT(level,"[APP_PS] "fmt, ##__VA_ARGS__)
#define APP_PS_PRNT(fmt,...) SYS_CONSOLE_PRINT("[APP_PS] "fmt, ##__VA_ARGS__)

// *****************************************************************************
    
// *****************************************************************************
typedef enum {

    WIFI_WSM_ON=0,
    WIFI_WSM_OFF,
    WIFI_WOFF,
    WIFI_WON,
} WIFI_SLEEP_MODE;

// *****************************************************************************

void APP_SetSleepMode(uint8_t);

#endif /* _APP_PS_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

