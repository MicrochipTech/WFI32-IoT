/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_wifi_prov.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/
    
#ifndef _APP_WIFI_PROV_H
#define _APP_WIFI_PROV_H

// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif
// DOM-IGNORE-END

// *****************************************************************************

#define APP_WIFI_PROV_DBG(level,fmt,...) SYS_DEBUG_PRINT(level,"[APP_WIFI_PROV] "fmt,##__VA_ARGS__)
#define APP_WIFI_PROV_PRNT(fmt,...) SYS_CONSOLE_PRINT("[APP_WIFI_PROV] "fmt, ##__VA_ARGS__)
    
#define APP_WIFI_PROV_WIFI_CONFIG_ID       "apply"
#define APP_WIFI_PROV_DONE_ID              "finish"
 
// *****************************************************************************

typedef enum
{
    /* Application WiFi prov task state machine*/
    APP_WIFI_PROV_INIT=0,
    APP_WIFI_PROV_PENDING,
    APP_WIFI_PROV_AP_ENABLE,
    APP_WIFI_PROV_WAITING_FOR_AP_ENABLED,
    APP_WIFI_PROV_AP_ENABLED,
    APP_WIFI_PROV_AP_DISABLE,
    APP_WIFI_PROV_WAITING_FOR_AP_DISABLED,
    APP_WIFI_PROV_IDLE,
    APP_WIFI_PROV_ERROR
} APP_TASK_WIFI_PROV_STATES;

typedef enum
{
    /* Application TCP server task state machine. */
    APP_TCP_SERVER_INIT=0,
    APP_TCP_SERVER_PENDING,
    APP_TCP_SERVER_OPEN_SOCKET,
    APP_TCP_SERVER_WAITING_SOCKET_CONNECTION,
    APP_TCP_SERVER_PARSE_SOCKET_DATA,
    APP_TCP_SERVER_CLOSE_SOCKET,
    APP_TCP_SERVER_IDLE,
    APP_TCP_SERVER_ERROR
} APP_TASK_TCP_SERVER_STATES;

// *****************************************************************************

typedef struct
{
    /* The application's current state */
	APP_TASK_TCP_SERVER_STATES wifiProvTaskState;
    APP_TASK_TCP_SERVER_STATES tcpServerTaskState;
    bool apReady;
    bool isConnected;
    TCP_SOCKET socket;
    uint8_t appBuffer[256];
} APP_WIFI_PROV_DATA;
APP_WIFI_PROV_DATA appWifiProvData;


// *****************************************************************************

void APP_InitializeWifiProv ( void );
void APP_TaskWifiProv( void );
void APP_TaskTcpServer ( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_WIFI_PROV_H */

/*******************************************************************************
 End of File
 */
