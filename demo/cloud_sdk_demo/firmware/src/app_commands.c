/*******************************************************************************
  Sample Application

  File Name:
    app_commands.c

  Summary:
    commands for the tcp client demo app.

  Description:
    
 *******************************************************************************/


// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END

#include "tcpip/tcpip.h"
#include "app.h"
#include "app_common.h"
#include "app_commands.h"
#include "app_ctrl.h"
#include "app_usb_msd.h"
#include "app_oled.h"
#include "app_ps.h"
#include "config.h"
#include <wolfssl/ssl.h>
#include "task.h"
#include "wolfcrypt/error-crypt.h"
#include "cryptoauthlib.h"
#include "wdrv_pic32mzw_common.h"
#include "wdrv_pic32mzw_assoc.h"
#include "system/debug/sys_debug.h"

//******************************************************************************

#if defined(TCPIP_STACK_COMMAND_ENABLE)

static void _APP_Commands_GetUnixTime(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_GetRSSI(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_GetRTCC(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_SetRTCCFreq(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_SelfTester(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_SetDebugLevel(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_SetPowerMode(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_Reboot(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);

//******************************************************************************

static const SYS_CMD_DESCRIPTOR appCmdTbl[] = {
    {"unixtime", _APP_Commands_GetUnixTime, ": Unix Time"},
    {"rssi", _APP_Commands_GetRSSI, ": Get current RSSI"},
    {"rtcc", _APP_Commands_GetRTCC, ": Get uptime"},
    {"rtcc_freq", _APP_Commands_SetRTCCFreq, ": Set RTCC frequency"},
    {"power_mode", _APP_Commands_SetPowerMode, ": Set power mode"},
    {"self_tester", _APP_Commands_SelfTester, ": Show board self tester status"},
    {"debug", _APP_Commands_SetDebugLevel, ": Set debug level"},
    {"reboot", _APP_Commands_Reboot, ": System reboot"},
};

//******************************************************************************

static void rssiCallback(DRV_HANDLE handle, WDRV_PIC32MZW_ASSOC_HANDLE assocHandle, int8_t rssi) {
    APP_CMD_PRNT("Connected RSSI: %d \r\n", rssi);
}

//******************************************************************************

 bool APP_Commands_Init() {
    if (!SYS_CMD_ADDGRP(appCmdTbl, sizeof (appCmdTbl) / sizeof (*appCmdTbl), "app", ": app commands")) {
        SYS_ERROR(SYS_ERROR_ERROR, "Failed to create TCPIP Commands\r\n", 0);
        return false;
    }
    return true;
}

void _APP_Commands_SetDebugLevel(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    if (argc == 2){
        uint8_t debugLevel = atoi(argv[1]);
        if(SYS_ERROR_FATAL <= debugLevel && debugLevel <= SYS_ERROR_DEBUG) {
            SYS_DEBUG_ErrorLevelSet((SYS_ERROR_LEVEL)debugLevel);
            APP_CMD_PRNT("Debug level set to %d \r\n", debugLevel);
            return;
        }
    }
    APP_CMD_PRNT("debug <level>\r\n");
}

void _APP_Commands_SetPowerMode(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    if (argc == 3){
        uint8_t a = atoi(argv[1]); 
        uint8_t b = atoi(argv[2]);
        APP_SetSleepMode(a,b);
        return;
    }
    APP_CMD_PRNT("power_mode <PIC_PM_MODE> <WIFI_PM_MODE>\r\n"
            "a b : PIC PM + Wi-Fi PM  \r\n"
            "0 0 : IDLE   + WSM_ON    \r\n"
            "0 2 : IDLE   + WOFF      \r\n"
            "1 0 : SLEEP  + WSM_ON    \r\n"
            "1 2 : SLEEP  + WOFF      \r\n"
            "3 - : DS/XDS +   -       \r\n"
            "4 3 :    -   + WON       \r\n");
}

void _APP_Commands_Reboot(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    APP_CMD_PRNT("*** System Reboot *** \r\n");
    vTaskDelay(100);
    APP_SoftResetDevice();
}

void _APP_Commands_GetRSSI(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    if (WIFI_IS_CONNECTED) {
        WDRV_PIC32MZW_STATUS ret;
        ret = WDRV_PIC32MZW_AssocRSSIGet(appData.assocHandle, NULL, rssiCallback);
        if (ret != WDRV_PIC32MZW_STATUS_RETRY_REQUEST) {
            APP_CMD_PRNT("Failed getting RSSI. Driver error (%d)\r\n", ret);
        }
    } else {
        APP_CMD_PRNT("RSSI: WI-Fi not connected.\r\n");
    }
}

void _APP_Commands_SetRTCCFreq(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    RTCC_ALARM_MASK mask;
    if (argc == 2){
        uint8_t freq = atoi(argv[1]);    
        if((RTCC_ALARM_MASK_HALF_SECOND <= freq && freq <= RTCC_ALARM_MASK_YEAR)
                || freq == RTCC_ALARM_MASK_OFF) {
            APP_CMD_PRNT("Setting RTCC frequency %x\r\n", freq);
            mask = freq;
            if (RTCC_AlarmSet(&appCtrlData.rtccData.sysTime, mask) == false) {
                APP_CTRL_DBG(SYS_ERROR_ERROR, "Error setting alarm\r\n");
            }
            return;
        }
    }
    APP_CMD_PRNT("rtcc_freq <freq> \r\n"
            "Half sec: 0 \r\n"
            "Sec: 1 \r\n"
            "10 sec: 2 \r\n"
            "Min: 3 \r\n"
            "10 min: 4 \r\n"
            "Hour: 5 \r\n"
            "Day: 6 \r\n"
            "Week: 7 \r\n"
            "Month: 8 \r\n"
            "Year: 9 \r\n"
            "OFF: 255 \r\n");
}

void _APP_Commands_GetRTCC(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    struct tm *sysTime = &appCtrlData.rtccData.sysTime;
    APP_CMD_PRNT("RTCC: %d-%d-%d %d:%d:%d\r\n", sysTime->tm_mday, sysTime->tm_mon, sysTime->tm_year, sysTime->tm_hour, sysTime->tm_min, sysTime->tm_sec);
}

void _APP_Commands_GetUnixTime(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    uint32_t sec = TCPIP_SNTP_UTCSecondsGet();
    APP_CMD_PRNT("Time from SNTP: %d\r\n", sec);
    APP_CMD_PRNT("Low Rez Timer: %d\r\n", SYS_TIME_CounterGet() /
            SYS_TIME_FrequencyGet());
}
void _APP_Commands_SelfTester(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv) {
    #define NUM_OF_TEST_LINE    5 
    uint8_t i = 0;
    struct testEntry{char* a; char* b;};
    
    /* Test items*/
    struct testEntry testStream[NUM_OF_TEST_LINE] = {
        {"ECC608            ", (appUSBMSDData.ecc608SerialNum > 0)? "PASS":"FAIL"},
        {"SPI Flash         ", (appUSBMSDData.fsMounted)?           "PASS":"FAIL"},
        {"MCP9809           ", (appCtrlData.mcp9808.deviceID > 0)?     "PASS":"FAIL"},
        {"OPT3001           ", (appCtrlData.opt3001.deviceID > 0)?     "PASS":"FAIL"},
        {"Click Interface   ", (appOLEDData.loopback)?              "PASS":"FAIL"},
    };
    
    /* Print */
    for(i=0; i< NUM_OF_TEST_LINE; i++)
        APP_CMD_PRNT("%s: %s\r\n", testStream[i].a, testStream[i].b);
}


#endif
