/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_usb_msd.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_USB_MSD_H
#define _APP_USB_MSD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"
#include "usb/usb_chapter_9.h"
#include "usb/usb_device.h"
#include "system/fs/sys_fs.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************

/* Debug wrappers */
#define APP_USB_MSD_DBG(level,fmt,...) SYS_DEBUG_PRINT(level,"[APP_USBMSD] "fmt,##__VA_ARGS__)
#define APP_USB_MSD_PRNT(fmt,...) SYS_CONSOLE_PRINT("[APP_USBMSD] "fmt, ##__VA_ARGS__)
 
/* Config/Web files' names */
#define APP_USB_MSD_WIFI_CONFIG_FILE_NAME   "WIFI.CFG"    
#define APP_USB_MSD_CLOUD_CONFIG_FILE_NAME  "cloud.json"
#define APP_USB_MSD_DEVCERT_FILE_NAME       "%s.cer"
#define APP_USB_MSD_CLICKME_FILE_NAME       "clickme.html"
#define APP_USB_MSD_VOICE_CLICKME_FILE_NAME "voice.html"
#define APP_USB_MSD_KIT_INFO_FILE_NAME      "kit-info.html"

/* Config/Web files' contents */
#define APP_USB_MSD_WIFI_CONFIG_ID              "CMD:SEND_UART=wifi"
#define APP_USB_MSD_WIFI_CONFIG_DATA_TEMPLATE   APP_USB_MSD_WIFI_CONFIG_ID" %s,%s,%d"
#define APP_USB_MSD_WIFI_CONFIG_MIN_LEN         (strlen(APP_USB_MSD_WIFI_CONFIG_ID) \
                                                + 1 \
                                                + sizeof(WIFI_AUTH))
#define APP_USB_MSD_AZURE_CLOUD_CONFIG_DATA_TEMPLATE "{\r\n\"Endpoint\":\"%s\",\r\n\"ThmbPrnt\":\"%s\"\r\n}"
#define APP_USB_MSD_AWS_CLOUD_CONFIG_DATA_TEMPLATE "{\r\n\"Endpoint\":\"%s\",\r\n\"ClientID\":\"%s\"\r\n}"
#define APP_USB_MSD_CLICKME_DATA_TEMPLATE "<html><body><script type=\"text/javascript\">window.location.href =\"\
                              https://iot.microchip.com/pic32mzw1/aws/%s\";</script></body></html>"
#define APP_USB_MSD_VOICE_CLICKME_DATA_TEMPLATE "<html><body><script type=\"text/javascript\">window.location.href =\"\
                              https://microchiptech.github.io/mchpiotvoice?thingName=%s&boardType=w1Curiosity\";</script></body></html>"
#define APP_USB_MSD_KIT_INFO_DATA_TEMPLATE "<html><body><script type=\"text/javascript\">window.location.href =\"\
                              https://www.microchip.com/en-us/development-tool/EV36W50A\";</script></body></html>"

#if !SYS_FS_AUTOMOUNT_ENABLE
    #define SYS_FS_MEDIA_IDX0_MOUNT_NAME_VOLUME_IDX0 			"/mnt/myDrive1"
    #define SYS_FS_MEDIA_IDX0_DEVICE_NAME_VOLUME_IDX0			"/dev/mtda1"
#endif
    
#define APP_USB_MSD_DRIVE_NAME              "CURIOSITY"
// *****************************************************************************

typedef enum
{
    /* Application USB MSD task state machine. */
    APP_USB_MSD_INIT=0,
    APP_USB_MSD_PENDING,
    APP_USB_MSD_WAIT_FS_MOUNT,
    APP_USB_MSD_CLEAR_DRIVE,
    APP_USB_MSD_TOUCH_CLOUD_FILES,
    APP_USB_MSD_TOUCH_WIFI_CONFIG_FILE,
    APP_USB_MSD_CONNECT,
    APP_USB_MSD_FS_UNMOUNT,
    APP_USB_MSD_DEINIT,
    APP_USB_MSD_IDLE,
    APP_USB_MSD_ERROR
} APP_TASK_USB_MSD_STATES;

// *****************************************************************************

typedef struct
{
    APP_TASK_USB_MSD_STATES USBMSDTaskState;
    /* USB Device Handle */
    USB_DEVICE_HANDLE usbDeviceHandle; 
    /* SYS_FS File handle */
    SYS_FS_HANDLE fileHandle;
    SYS_FS_FSTAT fileStatus;
    volatile bool fsMounted;
    bool wifiConfigRewrite;
    uint8_t appBuffer[256];
    char ecc608SerialNum[27];
} APP_USB_MSD_DATA;
APP_USB_MSD_DATA appUSBMSDData;

// *****************************************************************************

void APP_SoftResetDevice(void);
void APP_RewriteWifiConfigFile(void);
void APP_USB_MSD_Initialize ( void );
void APP_USB_MSD_Tasks ( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_USB_MSD_H */

/*******************************************************************************
 End of File
 */
