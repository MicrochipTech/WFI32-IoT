/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_oled.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_OLED_Initialize" and "APP_OLED_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_OLED_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_OLED_H
#define _APP_OLED_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END
    
/* Debug wrappers */
#define APP_OLED_DBG(level,fmt,...) SYS_DEBUG_PRINT(level,"[APP_OLED] "fmt,##__VA_ARGS__)
#define APP_OLED_PRNT(fmt,...) SYS_CONSOLE_PRINT("[APP_OLED] "fmt, ##__VA_ARGS__)

// *****************************************************************************

typedef enum
{
    APP_OLED_PARAM_WIFI=0,
    APP_OLED_PARAM_CLOUD
} APP_OLED_NOTIFY_TYPE;
    
typedef enum
{
    /* Application's state machine's initial state. */
    APP_OLED_STATE_INIT=0,
    APP_OLED_STATE_IDLE,
    APP_OLED_STATE_RUNNING
} APP_OLED_STATES;

typedef struct
{
    /* The application's current state */
    APP_OLED_STATES state;
    bool wifiCloudConnected[2];
    bool loopback;
} APP_OLED_DATA;
APP_OLED_DATA appOLEDData;

// *****************************************************************************

void APP_OLEDNotify ( APP_OLED_NOTIFY_TYPE, bool );
void APP_OLED_Initialize ( void );
void APP_OLED_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_OLED_H */

/*******************************************************************************
 End of File
 */
