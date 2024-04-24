/*******************************************************************************
  User Configuration Header

  File Name:
    user.h

  Summary:
    Build-time configuration header for the user defined by this project.

  Description:
    An MPLAB Project may have multiple configurations.  This file defines the
    build-time options for a single configuration.

  Remarks:
    It only provides macro definitions for build-time configuration options

*******************************************************************************/

#ifndef USER_H
#define USER_H

#include "peripheral/gpio/plib_gpio.h"


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: User Configuration macros
// *****************************************************************************
// *****************************************************************************
#define WOLFSSL_DER_TO_PEM
#define WOLFSSL_BASE64_ENCODE


#define LED_RED_On    LED_RED_Clear
#define LED_YELLOW_On LED_YELLOW_Clear
#define LED_GREEN_On  LED_GREEN_Clear
#define LED_BLUE_On   LED_BLUE_Clear

#define LED_RED_Off    LED_RED_Set
#define LED_YELLOW_Off LED_YELLOW_Set
#define LED_GREEN_Off  LED_GREEN_Set
#define LED_BLUE_Off   LED_BLUE_Set
    
//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif // USER_H
/*******************************************************************************
 End of File
*/
