/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_H
#define _APP_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "definitions.h"
#include "FreeRTOS.h"
#include "task.h"
// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************

#define APP_DBG(level,fmt,...) SYS_DEBUG_PRINT(level,"[APP] "fmt,##__VA_ARGS__)
#define APP_PRNT(fmt,...) SYS_CONSOLE_PRINT("[APP] "fmt, ##__VA_ARGS__)

// *****************************************************************************

typedef enum 
{
    /* Application's state machine's initial state. */
    APP_WLAN_LEDS_START_UP_PATTERN = 0,
    APP_WLAN_INIT,
    APP_WLAN_WDRV_INIT_READY,
    APP_WLAN_WAIT_FOR_TCPIP_INIT,
    APP_WLAN_CHECK_CREDENTIALS,
    APP_WLAN_CONFIG,
    APP_WLAN_WAIT_FOR_SNTP_INIT,    
    APP_WLAN_IDLE,
    APP_WLAN_RECONNECT,
    APP_WLAN_ERROR,
} APP_TASK_WLAN_STATES;

typedef enum 
{
    /* Application's state machine's initial state. */
    APP_MODE_STA = 0,
    APP_MODE_AP
} APP_MODE;

// *****************************************************************************
/* AP parameters

  Summary:
     Settings related o wireless configuration  
*/


#define DEFAULT_SSID "WFI32-IoT_000000000000"
#define DEFAULT_SSID_PSK "1234567890"
#define DEFAULT_AUTH_MODE (WIFI_AUTH)OPEN
#define DEVICE_CHANNEL WDRV_PIC32MZW_CID_2_4G_CH1
#define WLAN_SSID_VISIBLE

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* The application's current state */
    APP_TASK_WLAN_STATES wlanTaskState;

    /* TODO: Define any additional data used by the application. */
    DRV_HANDLE wdrvHandle;
    uintptr_t assocHandle;
    volatile bool isRegDomainSet;
    volatile bool isConnected;
    volatile bool isIPObtained;
    volatile bool isNTPDone;
    volatile bool sw1Pressed;
    volatile bool sw2Pressed;
    APP_MODE appMode;
    TCPIP_EVENT_HANDLE TCPIPEventHandle;
    TCPIP_DHCP_HANDLE TCPIPDHCPHandle;
    
    uint8_t ValidCrednetials;

} APP_DATA;
APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/
void *APP_Calloc(size_t num, size_t size);

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************
void APP_InitializeWlan ( void );
/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_Tasks( void );



#endif /* _APP_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

