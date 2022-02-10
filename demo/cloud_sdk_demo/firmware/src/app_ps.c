/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_ctrl.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

#include "app_common.h"
#include "app_ps.h"

// *****************************************************************************
WIFI_SLEEP_MODE wifiPsMode = WIFI_WON;

// *****************************************************************************


// *****************************************************************************


static void clearINTSource()
{
    SYS_INT_SourceStatusClear(INT_SOURCE_USB);
    
    SYS_INT_SourceStatusClear(INT_SOURCE_CORE_TIMER);
    
    SYS_INT_SourceStatusClear(INT_SOURCE_I2C2_MASTER);
    SYS_INT_SourceStatusClear(INT_SOURCE_I2C2_BUS);
    
    SYS_INT_SourceStatusClear(INT_SOURCE_SPI2_FAULT);
    SYS_INT_SourceStatusClear(INT_SOURCE_SPI2_RX);
    SYS_INT_SourceStatusClear(INT_SOURCE_SPI2_TX);
    
    SYS_INT_SourceStatusClear(INT_SOURCE_SPI1_FAULT);
    SYS_INT_SourceStatusClear(INT_SOURCE_SPI1_RX);
    SYS_INT_SourceStatusClear(INT_SOURCE_SPI1_TX);
    
    SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_1);
    SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_3);

    SYS_INT_SourceStatusClear(INT_SOURCE_UART1_FAULT);
    SYS_INT_SourceStatusClear(INT_SOURCE_UART1_TX);
    SYS_INT_SourceStatusClear(INT_SOURCE_UART1_RX);

    SYS_INT_SourceStatusClear(INT_SOURCE_UART3_FAULT);
    SYS_INT_SourceStatusClear(INT_SOURCE_UART3_TX);
    SYS_INT_SourceStatusClear(INT_SOURCE_UART3_RX);

    SYS_INT_SourceStatusClear(INT_SOURCE_I2C1_MASTER);
    SYS_INT_SourceStatusClear(INT_SOURCE_I2C1_BUS);

    SYS_INT_SourceStatusClear(INT_SOURCE_RFMAC);
    SYS_INT_SourceStatusClear(INT_SOURCE_RFSMC);
    SYS_INT_SourceStatusClear(INT_SOURCE_RFTM0);

    SYS_INT_SourceStatusClear(INT_SOURCE_CRYPTO1);
    SYS_INT_SourceStatusClear(INT_SOURCE_CRYPTO1_FAULT);

    SYS_INT_SourceStatusClear(INT_SOURCE_FLASH_CONTROL);  
    //SYS_INT_SourceStatusClear(INT_SOURCE_RTCC);
    
    //SYS_INT_SourceStatusClear(INT_SOURCE_EXTERNAL_0);
    //SYS_INT_SourceStatusClear(INT_SOURCE_CHANGE_NOTICE_A);
}

static void disableINTSource()
{
    SYS_INT_SourceDisable(INT_SOURCE_USB);
    
    SYS_INT_SourceDisable(INT_SOURCE_CORE_TIMER);
    
    SYS_INT_SourceDisable(INT_SOURCE_I2C2_MASTER);
    SYS_INT_SourceDisable(INT_SOURCE_I2C2_BUS);
    
    SYS_INT_SourceDisable(INT_SOURCE_SPI2_FAULT);
    SYS_INT_SourceDisable(INT_SOURCE_SPI2_RX);
    SYS_INT_SourceDisable(INT_SOURCE_SPI2_TX);

    SYS_INT_SourceDisable(INT_SOURCE_SPI1_FAULT);
    SYS_INT_SourceDisable(INT_SOURCE_SPI1_RX);
    SYS_INT_SourceDisable(INT_SOURCE_SPI1_TX);
    
    SYS_INT_SourceDisable(INT_SOURCE_TIMER_1);
    SYS_INT_SourceDisable(INT_SOURCE_TIMER_3);

    SYS_INT_SourceDisable(INT_SOURCE_UART1_FAULT);
    SYS_INT_SourceDisable(INT_SOURCE_UART1_TX);
    SYS_INT_SourceDisable(INT_SOURCE_UART1_RX);

    SYS_INT_SourceDisable(INT_SOURCE_UART3_FAULT);
    SYS_INT_SourceDisable(INT_SOURCE_UART3_TX);
    SYS_INT_SourceDisable(INT_SOURCE_UART3_RX);

    SYS_INT_SourceDisable(INT_SOURCE_I2C1_MASTER);
    SYS_INT_SourceDisable(INT_SOURCE_I2C1_BUS);

    SYS_INT_SourceDisable(INT_SOURCE_RFMAC);
    SYS_INT_SourceDisable(INT_SOURCE_RFSMC);
    SYS_INT_SourceDisable(INT_SOURCE_RFTM0);

    SYS_INT_SourceDisable(INT_SOURCE_CRYPTO1);
    SYS_INT_SourceDisable(INT_SOURCE_CRYPTO1_FAULT);

    SYS_INT_SourceDisable(INT_SOURCE_FLASH_CONTROL);
    //SYS_INT_SourceDisable(INT_SOURCE_RTCC);
    
    //SYS_INT_SourceDisable(INT_SOURCE_EXTERNAL_0);
    //SYS_INT_SourceDisable(INT_SOURCE_CHANGE_NOTICE_A);
    
}

static void restoreINTSource()
{
    SYS_INT_SourceRestore(INT_SOURCE_USB, true);
    
    SYS_INT_SourceRestore(INT_SOURCE_CORE_TIMER, true);
    
    SYS_INT_SourceRestore(INT_SOURCE_I2C2_MASTER,true);
    SYS_INT_SourceRestore(INT_SOURCE_I2C2_BUS, true);
    
    SYS_INT_SourceRestore(INT_SOURCE_SPI2_FAULT,true);
    SYS_INT_SourceRestore(INT_SOURCE_SPI2_RX, true);
    SYS_INT_SourceRestore(INT_SOURCE_SPI2_TX, true);
    
    SYS_INT_SourceRestore(INT_SOURCE_SPI1_FAULT,true);
    SYS_INT_SourceRestore(INT_SOURCE_SPI1_RX, true);
    SYS_INT_SourceRestore(INT_SOURCE_SPI1_TX, true);

    SYS_INT_SourceRestore(INT_SOURCE_TIMER_1, true);
    SYS_INT_SourceRestore(INT_SOURCE_TIMER_3, true);

    SYS_INT_SourceRestore(INT_SOURCE_UART1_FAULT, true);
    SYS_INT_SourceRestore(INT_SOURCE_UART1_TX, true);
    SYS_INT_SourceRestore(INT_SOURCE_UART1_RX, true);

    SYS_INT_SourceRestore(INT_SOURCE_UART3_FAULT, true);
    SYS_INT_SourceRestore(INT_SOURCE_UART3_TX, true);
    SYS_INT_SourceRestore(INT_SOURCE_UART3_RX, true);

    SYS_INT_SourceRestore(INT_SOURCE_I2C1_MASTER, true);
    SYS_INT_SourceRestore(INT_SOURCE_I2C1_BUS, true);

    SYS_INT_SourceRestore(INT_SOURCE_RFMAC, true);
    SYS_INT_SourceRestore(INT_SOURCE_RFSMC, true);
    SYS_INT_SourceRestore(INT_SOURCE_RFTM0, true);

    
    SYS_INT_SourceRestore(INT_SOURCE_CRYPTO1, true);
    SYS_INT_SourceRestore(INT_SOURCE_CRYPTO1_FAULT, true);

    SYS_INT_SourceRestore(INT_SOURCE_FLASH_CONTROL, true);
    //SYS_INT_SourceRestore(INT_SOURCE_RTCC, true);

    //SYS_INT_SourceRestore(INT_SOURCE_EXTERNAL_0, true);
    //SYS_INT_SourceRestore(INT_SOURCE_CHANGE_NOTICE_A, true);
}

void APP_SetSleepMode(POWER_LOW_POWER_MODE picSM, WIFI_SLEEP_MODE wifiSM)
{
    #define LOW_POWER_NONE  LOW_POWER_DEEP_SLEEP_MODE+1

    /* Set RTCC alarm to 10 seconds */
    if (RTCC_AlarmSet(&appCtrlData.rtccData.sysTime, RTCC_ALARM_MASK_10_SECONDS) == false) {
        APP_CTRL_DBG(SYS_ERROR_ERROR, "Error setting alarm\r\n");
        return;
    }
    
    switch (picSM) {
        /* PIC DS or XDS mode */
        case LOW_POWER_DEEP_SLEEP_MODE:
            PMD3bits.W24GMD = 1;
            POWER_LowPowerModeEnter(picSM);
            break;

        /* In PIC sleep & idle modes, Wi-Fi must be in WOFF or WSM */
        case LOW_POWER_SLEEP_MODE:           
        case LOW_POWER_IDLE_MODE:
            if((wifiSM != WIFI_WOFF && wifiSM != WIFI_WSM_ON)
                    || !MQTT_IS_CONNECTED){
                APP_PS_PRNT("Invalid PM configuration or MQTT is not connected\r\n");
                break;
            }
            
            if(wifiSM == WIFI_WOFF){
                PMUCLKCTRLbits.WLDOOFF = 1;
            }
            
            clearINTSource();
            disableINTSource();
            POWER_LowPowerModeEnter(picSM);
            restoreINTSource();
            
            if(wifiSM == WIFI_WOFF){
                WDRV_PIC32MZW_Deinitialize(sysObj.drvWifiPIC32MZW1);
                appData.wOffRequested = true;
            }
            break;
            
        /* Wi-Fi WON */
        case LOW_POWER_NONE:
            if(MQTT_IS_CONNECTED || wifiSM != WIFI_WON){
                APP_PS_PRNT("Invalid PM configuration or MQTT is connected\r\n");
                break;
            }         
            appData.wOnRequested = true;
            break;
        
        default:
            APP_PS_PRNT("Invalid PM configuration\r\n");
            break;
    }
}

/*******************************************************************************
 End of File
 */
