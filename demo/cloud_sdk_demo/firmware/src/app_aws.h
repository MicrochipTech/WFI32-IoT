/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_aws.h

  Summary:
    This header file provides prototypes and definitions for the application AWS part.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_AWS_H
#define _APP_AWS_H

#ifdef AWS_CLOUD_DEMO

// *****************************************************************************

#include "iot_platform_types_pic32mzw1.h"
#include "iot_mqtt.h"
#include "configuration.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
    
#define APP_AWS_DBG(level,fmt,...) SYS_DEBUG_PRINT(level,"[APP_AWS] "fmt,##__VA_ARGS__)
#define APP_AWS_PRNT(fmt,...) SYS_CONSOLE_PRINT("[APP_AWS] "fmt, ##__VA_ARGS__)
    
#define APP_USE_X509_CERT   
#define APP_AWS_TOPIC_NAME_MAX_LEN            128
#define APP_AWS_TELEMETRY_MSG_TEMPLATE "{\"Temperature (C)\": %d,\"Light (lux)\":%d}"
#define APP_AWS_TELEMETRY_MSG_GRAD_TEMPLATE "{\"Temperature (C)\": %d,\"Light (lux)\":%d,\"Switch 1\":%d}"
#define APP_AWS_SHADOW_MSG_TEMPLATE "{\"state\":{\"reported\":{\"toggle\": %d}}}"
#define APP_AWS_MAX_MSG_LLENGTH 64
#define APP_AWS_SHADOW_UPDATE_TOPIC_TEMPLATE "$aws/things/%s/shadow/update"
#define APP_AWS_SHADOW_DELTA_TOPIC_TEMPLATE "$aws/things/%s/shadow/update/#"
#define PUBLISH_FREQUENCY_MS       1000

// *****************************************************************************

typedef enum
{
    /* Application AWS cloud state machine */
    APP_AWS_CLOUD_SDK_INIT,
    APP_AWS_CLOUD_PENDING,
    APP_AWS_CLOUD_MQTT_CONNECT,
    APP_AWS_CLOUD_MQTT_PUBLISH_TO_TOPIC,
    APP_AWS_CLOUD_MQTT_SUBSCRIBE_TO_TOPIC,
    APP_AWS_CLOUD_IDLE,            
    APP_AWS_CLOUD_ERROR
} APP_TASK_AWS_CLOUD_STATES;

// *****************************************************************************

typedef struct
{
    /* The application's current state */
    APP_TASK_AWS_CLOUD_STATES awsCloudTaskState;
    /* Handle of the MQTT connection used in this demo. */
    IotMqttConnection_t mqttConnection;
    /* MQTT connection status */
    bool mqttConnected;
    /* shadow update */
    bool shadowUpdate;
    /* Timer to take care of publishing to cloud */
    SYS_TIME_HANDLE pubTimerHandle;
    bool publishToCloud;
    /* Track number of messages sent without getting a callback for */
    uint8_t pendingMessages;
} APP_AWS_DATA;
APP_AWS_DATA appAwsData;

// *****************************************************************************

void APP_AWS_Initialize( void );
void APP_AWS_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* AWS_CLOUD_DEMO */
#endif /* _APP_AWS_H */

/*******************************************************************************
 End of File
 */
