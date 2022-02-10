/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ctrl.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_CTRL_H
#define _APP_CTRL_H

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

#define APP_CTRL_DBG(level,fmt,...) SYS_DEBUG_PRINT(level,"[APP_CTRL] "fmt, ##__VA_ARGS__)
#define APP_CTRL_PRNT(fmt,...) SYS_CONSOLE_PRINT("[APP_CTRL] "fmt, ##__VA_ARGS__)
#define NUM_OF_LEDS                     4

// *****************************************************************************
    
// *****************************************************************************

/* LEDs */
typedef enum 
{
    LED_RED = 0,
    LED_GREEN,
    LED_YELLOW,
    LED_BLUE,
    LED_COLOR_IVALID,
} LED_COLOR;

/* LED mode */
typedef enum 
{
    LED_ON = 0,
    LED_OFF,
    LED_TOGGLE,
    LED_F_BLINK,                  /* A fast blink (always started in 'on' state) */
    LED_S_BLINK_STARTING_ON,      /* A slow blink started in 'on' state */
    LED_S_BLINK_STARTING_OFF,     /* A slow blink started in 'off' state */
    LED_MODE_INVALID,
} LED_MODE;

/* Blinking LED mode */
typedef enum 
{
    BLINK_MODE_SINGLE = 0,
    BLINK_MODE_PERIODIC,
    BLINK_MODE_INVALID,
} LED_BLINK_MODE;

/* I2C tranfser status */
typedef enum
{
    I2C_TRANSFER_STATUS_IN_PROGRESS,
    I2C_TRANSFER_STATUS_SUCCESS,
    I2C_TRANSFER_STATUS_ERROR,
    I2C_TRANSFER_STATUS_IDLE,
} APP_CTRL_I2C_TRANSFER_STATUS;

/* Application CTRL task state machine. */
typedef enum
{
    APP_CTRL_INIT=0,
    APP_CTRL_CHECK,
    APP_CTRL_TURN_ON_MCP9808,
    APP_CTRL_WAIT_TURN_ON_MCP9808,
    APP_CTRL_TURN_ON_OPT3001,
    APP_CTRL_WAIT_TURN_ON_OPT3001,
    APP_CTRL_SHUTDOWN_MCP9808,
    APP_CTRL_WAIT_SHUTDOWN_MCP9808,
    APP_CTRL_SHUTDOWN_OPT3001,
    APP_CTRL_WAIT_SHUTDOWN_OPT3001,
    APP_CTRL_READ_TEMP,
    APP_CTRL_WAIT_READ_TEMP,
    APP_CTRL_READ_LIGHT,
    APP_CTRL_WAIT_READ_LIGHT,
    APP_CTRL_READ_MCP9808_DEV_ID,
    APP_CTRL_WAIT_MCP9808_DEV_ID,
    APP_CTRL_READ_OPT3001_DEV_ID,
    APP_CTRL_WAIT_OPT3001_DEV_ID,
    APP_CTRL_IDLE,
    APP_CTRL_ERROR
} APP_TASK_CTRL_STATES;

/* Control structure periodic operations */
typedef struct
{
    uint16_t counter;
    uint16_t reload;
    bool periodic;
} APP_CTRL_TIMER_S;

/* I2C */
typedef struct
{
    DRV_HANDLE i2cHandle;
    DRV_I2C_TRANSFER_HANDLE transferHandle;
    APP_CTRL_I2C_TRANSFER_STATUS transferStatus;
    uint8_t txBuffer[4];
    uint16_t rxBuffer;
} APP_CTRL_I2C;

/* MCP9808 structure */
typedef struct
{
    bool IsShutdown;
    int16_t temperature;
    uint16_t deviceID;
} APP_CTRL_MCP9808;

/* OPT3001 structure */
typedef struct
{
    bool IsShutdown;
    uint32_t light;
    uint16_t deviceID;
} APP_CTRL_OPT3001;

/* RTCC data */
typedef struct {
    bool rtccAlarm;
    struct tm sysTime;
}APP_CTRL_RTCC;

/* APP CTRL module config structure */
typedef struct
{
    APP_TASK_CTRL_STATES ctrlTaskState;
    
    SYS_TIME_HANDLE timeHandle;
    APP_CTRL_TIMER_S ledBlinkCtrl[NUM_OF_LEDS];
    APP_CTRL_TIMER_S sensorsReadCtrl;
    bool readSensors;
    bool shutdownSensors;
    bool turnOnSensors;
    
    APP_CTRL_I2C i2c;
    
    APP_CTRL_MCP9808 mcp9808;
    APP_CTRL_OPT3001 opt3001;
    APP_CTRL_RTCC rtccData;
} APP_CTRL_DATA;
APP_CTRL_DATA appCtrlData;

// *****************************************************************************

void APP_sensorsOn(void);
void APP_sensorsOff(void);
int16_t APP_readTemp(void);
uint32_t APP_readLight(void);
void APP_manageLed(LED_COLOR, LED_MODE, LED_BLINK_MODE);
void APP_CTRL_Initialize (void);
void APP_CTRL_Tasks ( void );

#endif /* _APP_CTRL_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

