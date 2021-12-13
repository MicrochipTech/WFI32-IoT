/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_wifi_prov.c

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
#include "app.h"
#include "app_common.h"
#include "app_wifi_prov.h"
#include "wdrv_pic32mzw_client_api.h"

// *****************************************************************************

extern uint8_t g_macaddress[6];

// *****************************************************************************

#define SERVER_PORT 80
#define HEX2ASCII(x) (((x) >= 10) ? (((x) - 10) + 'A') : ((x) + '0'))
#define M2M_MAX_SSID_LEN 33
#define M2M_MAX_PSK_LEN  65

//#define AP_SSID_MAC_ADDR_TAILED

// *****************************************************************************

static void setDevParamToMac(uint8_t *param, uint8_t *addr)
{
	uint16_t len;

	len = strlen(param);
#ifndef AP_SSID_MAC_ADDR_TAILED
    *(char*)(&param[len - 13]) = '\0';
#else
    param[len - 1] = HEX2ASCII((addr[5] >> 0) & 0x0f);
    param[len - 2] = HEX2ASCII((addr[5] >> 4) & 0x0f);
    param[len - 3] = HEX2ASCII((addr[4] >> 0) & 0x0f);
    param[len - 4] = HEX2ASCII((addr[4] >> 4) & 0x0f);
    param[len - 5] = HEX2ASCII((addr[3] >> 0) & 0x0f);
    param[len - 6] = HEX2ASCII((addr[3] >> 4) & 0x0f);
    param[len - 7] = HEX2ASCII((addr[2] >> 0) & 0x0f);
    param[len - 8] = HEX2ASCII((addr[2] >> 4) & 0x0f);
    param[len - 9] = HEX2ASCII((addr[1] >> 0) & 0x0f);
    param[len - 10] = HEX2ASCII((addr[1] >> 4) & 0x0f);
    param[len - 11] = HEX2ASCII((addr[0] >> 0) & 0x0f);
    param[len - 12] = HEX2ASCII((addr[0] >> 4) & 0x0f);
#endif
}

/* Parse Wi-Fi configuration file */
/* Format is APP_WIFI_PROV_WIFI_CONFIG_ID,<SSID>,<AUTH>,<PASSPHRASE>*/
static int8_t parseWifiConfig()
{
    char* p;
    char* key;
    int8_t ret = 0;
    
    p = strtok((char *)appWifiProvData.appBuffer, ",");
    if (p != NULL && !strncmp(p, APP_WIFI_PROV_WIFI_CONFIG_ID, strlen(APP_WIFI_PROV_WIFI_CONFIG_ID))) {
        p = strtok(NULL, ",");
        if (p)
            strcpy(wifi.ssid, p);

        p = strtok(NULL, ",");
        if (p) 
        {
            wifi.auth = atoi(p);
            if (OPEN < wifi.auth &&  wifi.auth < WIFI_AUTH_MAX)
            {
                p = strtok(NULL, ",");
                if (p) 
                    strcpy((char *) wifi.key, p);
                else
                    ret = -1;
            } 
            else if (wifi.auth == OPEN)
                strcpy((char *) wifi.key, "");
            else
                ret = -1;
        }
        else
            ret = -1;

        APP_WIFI_PROV_DBG(SYS_ERROR_DEBUG, "SSID:%s - PASSPHRASE:%s - AUTH:%d\r\n", 
                            wifi.ssid, 
                            wifi.key, 
                            wifi.auth);
    }
    return ret;
}

// *****************************************************************************

void APP_InitializeWifiProv ( void )
{
    AP_DISCONNECTED;
    appWifiProvData.wifiProvTaskState = APP_WIFI_PROV_INIT;
    appWifiProvData.tcpServerTaskState = APP_TCP_SERVER_INIT;
}

void APP_TaskWifiProv ( void )
{
    bool status;
    switch ( appWifiProvData.wifiProvTaskState )
    {
        /* Application WiFi Prov task initial state. */
        case APP_WIFI_PROV_INIT:
        {
            appWifiProvData.wifiProvTaskState = APP_WIFI_PROV_PENDING;
            break;
        }

        /* Pending */
        case APP_WIFI_PROV_PENDING:
        {
            break;
        }
     
        /* Enable AP*/
        case APP_WIFI_PROV_AP_ENABLE:
        {
            uint8_t ssid[M2M_MAX_SSID_LEN] = DEFAULT_SSID;           
            WDRV_PIC32MZW_BSSCtxSetDefaults(&g_wifiConfig.bssCtx);
            setDevParamToMac(ssid, g_macaddress);
            status = APP_WifiConfig((char*)ssid, (char*)DEFAULT_SSID_PSK, (WIFI_AUTH)DEFAULT_AUTH_MODE, DEVICE_CHANNEL);
            if (!status) 
            {
                APP_WIFI_PROV_DBG(SYS_ERROR_ERROR, "Failed to write Wi-Fi credentials\r\n");
                appWifiProvData.wifiProvTaskState = APP_WIFI_PROV_ERROR;
                break;
            }

#ifdef WLAN_SSID_VISIBLE
            WDRV_PIC32MZW_BSSCtxSetSSIDVisibility(&g_wifiConfig.bssCtx, true);
#else            
            WDRV_PIC32MZW_BSSCtxSetSSIDVisibility(&bssCtx, false);
#endif            
            WDRV_PIC32MZW_APStart(appData.wdrvHandle, &g_wifiConfig.bssCtx, &g_wifiConfig.authCtx, NULL);
            
            appWifiProvData.wifiProvTaskState = APP_WIFI_PROV_WAITING_FOR_AP_ENABLED;
            break;
        }
        
        /* Wait for AP enabled */
        case APP_WIFI_PROV_WAITING_FOR_AP_ENABLED:
        {
            if(AP_IS_CONNECTED)
            {
               APP_WIFI_PROV_PRNT("AP is enabled\r\n");
               APP_manageLed(LED_BLUE, LED_S_BLINK_STARTING_ON, BLINK_MODE_PERIODIC);
                appWifiProvData.wifiProvTaskState = APP_WIFI_PROV_AP_ENABLED;          
                /* Change TCP server task state to open a socket*/
                appWifiProvData.tcpServerTaskState = APP_TCP_SERVER_OPEN_SOCKET;
            }
            break;
        }
        
        /* AP enabled */
        case APP_WIFI_PROV_AP_ENABLED:
        {   
            break;
        }
        
        /* Disable AP */
        case APP_WIFI_PROV_AP_DISABLE:
        {            
            if(WDRV_PIC32MZW_APStop(appData.wdrvHandle) != WDRV_PIC32MZW_STATUS_OK)
            {
                APP_WIFI_PROV_DBG(SYS_ERROR_ERROR, "Failed to stop AP\r\n");
            }
            APP_manageLed(LED_BLUE, LED_OFF, BLINK_MODE_INVALID);
            appWifiProvData.wifiProvTaskState = APP_WIFI_PROV_WAITING_FOR_AP_DISABLED;
            appWifiProvData.tcpServerTaskState = APP_TCP_SERVER_CLOSE_SOCKET;
            break;
        }
        
        /* Wait for AP enabled */
        case APP_WIFI_PROV_WAITING_FOR_AP_DISABLED:
        {
            if(!AP_IS_CONNECTED)
            {
                APP_WIFI_PROV_PRNT("AP is disabled\r\n");
                appWifiProvData.wifiProvTaskState = APP_WIFI_PROV_IDLE;            
            }  
            break;
        }

        /* Idle */
        case APP_WIFI_PROV_IDLE:
        {          
            break;
        }

        /* Error */
        case APP_WIFI_PROV_ERROR:
        {
            APP_WIFI_PROV_DBG(SYS_ERROR_ERROR, "APP_WIFI_PROV_ERROR\r\n");
            appWifiProvData.wifiProvTaskState = APP_WIFI_PROV_IDLE;
            break;
        }

        default:
        {
            break;
        }
    }
}

void APP_TaskTcpServer ( void )
{
    switch ( appWifiProvData.tcpServerTaskState )
    {
        /* TCP server task initial state. */
        case APP_TCP_SERVER_INIT:
        {
            appWifiProvData.tcpServerTaskState = APP_TCP_SERVER_PENDING;
            break;
        }
        
        /* TCP server task pending for WLAN task decision */
        case APP_TCP_SERVER_PENDING:
        {
            break;
        }
        
        /* Open a socket */
        case APP_TCP_SERVER_OPEN_SOCKET:
        {
            appWifiProvData.socket = TCPIP_TCP_ServerOpen(IP_ADDRESS_TYPE_IPV4, SERVER_PORT, 0);
            if(appWifiProvData.socket == INVALID_SOCKET)
            {
                APP_WIFI_PROV_DBG(SYS_ERROR_ERROR, "Couldn't open server socket\r\n");
                break;
            }
            APP_WIFI_PROV_DBG(SYS_ERROR_DEBUG, "TCP socket %d opened\r\n", appWifiProvData.socket);
            
            appWifiProvData.tcpServerTaskState = APP_TCP_SERVER_WAITING_SOCKET_CONNECTION;
            break;
        }
           
        /* Wait for a connection over this socket */
        case APP_TCP_SERVER_WAITING_SOCKET_CONNECTION:
        {
            if(!TCPIP_TCP_IsConnected(appWifiProvData.socket))
            {
                break;
            }
            APP_WIFI_PROV_PRNT("TCP socket %d connected\r\n", appWifiProvData.socket);
            
            appWifiProvData.tcpServerTaskState = APP_TCP_SERVER_PARSE_SOCKET_DATA;
            break;
        }
        
        /* Wait for data (valid credentials) over this socket*/
        case APP_TCP_SERVER_PARSE_SOCKET_DATA:
        {
            int readSize = 0;
            int8_t ret = 0;
            char *p;
            if(!TCPIP_TCP_IsConnected(appWifiProvData.socket))
            {
                return;
            }
            
            readSize = TCPIP_TCP_GetIsReady(appWifiProvData.socket);	// Get TCP RX FIFO byte count
            if(readSize)
            {
                if(readSize > sizeof(appWifiProvData.appBuffer) - 1)
                {
                    readSize = sizeof(appWifiProvData.appBuffer) - 1;
                }
                TCPIP_TCP_ArrayGet(appWifiProvData.socket, appWifiProvData.appBuffer, readSize);
                appWifiProvData.appBuffer[readSize] = '\0';
                APP_WIFI_PROV_DBG(SYS_ERROR_DEBUG, "Received command: len %d \r\n", readSize);
                APP_WIFI_PROV_DBG(SYS_ERROR_DEBUG, "%s \r\n", (char*)appWifiProvData.appBuffer);
                p = (char*)appWifiProvData.appBuffer;

                /* Check buffer contents for being Wi-Fi credentials*/
                ret = parseWifiConfig();
                if(ret < 0)
                {
                    APP_WIFI_PROV_DBG(SYS_ERROR_ERROR, "Failed parsing Wi-Fi config\r\n");
                    break;
                }
                
                /* Apply received Wi-Fi credentials */
                if (p != NULL && !strncmp(p, APP_WIFI_PROV_DONE_ID, strlen(APP_WIFI_PROV_DONE_ID))) {                                   
                    /* Store Wi-Fi credentials to MSD*/
                    APP_RewriteWifiConfigFile();
                    
                    APP_WIFI_PROV_PRNT("Provisioning complete\r\n");
                    appWifiProvData.tcpServerTaskState = APP_TCP_SERVER_CLOSE_SOCKET;
                    appWifiProvData.wifiProvTaskState = APP_WIFI_PROV_AP_DISABLE;
                }
            }
                        
            break;
        }
        
        /* Close the socket */
        case APP_TCP_SERVER_CLOSE_SOCKET:
        {            
            if(TCPIP_TCP_Close(appWifiProvData.socket) == false)
            {
                APP_WIFI_PROV_DBG(SYS_ERROR_ERROR, "Couldn't close server socket\r\n");
                break;
            }
            APP_WIFI_PROV_DBG(SYS_ERROR_DEBUG, "TCP socket %d closed\r\n", appWifiProvData.socket);
            
            appWifiProvData.tcpServerTaskState = APP_TCP_SERVER_IDLE;
            break;
        }
        
        /* Idle */
        case APP_TCP_SERVER_IDLE:
        {            
            break;
        }
        
        /* Error */
        case APP_TCP_SERVER_ERROR:
        {
            APP_WIFI_PROV_DBG(SYS_ERROR_ERROR, "APP_TCP_SERVER_ERROR\n");
            appWifiProvData.tcpServerTaskState = APP_TCP_SERVER_IDLE;
            break;
        }
        
        default:
        {
            break;
        }
    }
}

/*******************************************************************************
 End of File
 */
