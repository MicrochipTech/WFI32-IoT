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

#ifndef _OLEDB_H
#define _OLEDB_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "config/aws_sdk_wfi32_iot_freertos/driver/driver_common.h"


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif

typedef struct
{
    DRV_HANDLE spiHandle;
    bool status;
} OLEDB_DATA;

int oledb_initialize(bool*);
void oledb_clearDisplay(void);
void oledb_displayPicture(const uint8_t *pic);
void oledb_displayOff(void);
void oledb_displayOn(void);

#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _OLEDB_H */