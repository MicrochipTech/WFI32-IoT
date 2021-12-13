/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_aws.c

  Summary:
    This file contains the source code for the MPLAB Harmony application AWS part.

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

// *****************************************************************************

#ifdef AWS_CLOUD_DEMO

#include "iot_config.h"
#include "app_common.h"
#include "app_aws.h"
#include "app_oled.h"
#include "cJSON.h"
#include "iot_network_wolfssl.h"

// *****************************************************************************

char g_Cloud_Endpoint[100];
char g_Aws_ClientID[CLIENT_IDENTIFIER_MAX_LENGTH];

// *****************************************************************************

/* Publish to cloud every 'PUBLISH_FREQUENCY_MS' milliseconds */
static void pubTimerCallback(uintptr_t context) {
    appAwsData.publishToCloud = true;
}

/* MQTT disconnect callback */
static void mqttDisconnectCallback( void * param1,
                                        IotMqttCallbackParam_t * const pOperation )
{
    APP_AWS_DBG(SYS_ERROR_ERROR, "MQTT connection terminated \r\n");
    APP_manageLed(LED_GREEN, LED_OFF, BLINK_MODE_INVALID);
    APP_OLEDNotify(APP_OLED_PARAM_CLOUD, false);
    MQTT_DISCONNECTED;
}

/* Called by the MQTT library when an operation completes. */
static void operationCompleteCallback( void * param1,
                                        IotMqttCallbackParam_t * const pOperation )
{
    appAwsData.pendingMessages--;
    if( pOperation->u.operation.result == IOT_MQTT_SUCCESS )
    {
        APP_AWS_DBG(SYS_ERROR_INFO, "MQTT %s successfully sent \r\n",
                    IotMqtt_OperationType( pOperation->u.operation.type ));
        APP_manageLed(LED_YELLOW, LED_F_BLINK, BLINK_MODE_SINGLE);
    }
    else
    {
        APP_AWS_DBG(SYS_ERROR_ERROR, "MQTT %s could not be sent. Error %s \r\n",
                     IotMqtt_OperationType( pOperation->u.operation.type ),
                     IotMqtt_strerror( pOperation->u.operation.result ) );
    }
}

/* Publish message is received */
static void MqttCallback( void * param1,
                                       IotMqttCallbackParam_t * const pPublish )
{
    const char * pPayload = pPublish->u.message.info.pPayload;
    
    if (NULL != strstr(pPublish->u.message.info.pTopicName, "/shadow/update/delta")) {
        /* Print information about the incoming PUBLISH message. */
        APP_AWS_DBG(SYS_ERROR_DEBUG,  "Incoming PUBLISH received:\r\n"
                    "Subscription topic filter: %.*s\r\n"
                    "Publish topic name: %.*s\r\n"
                    "Publish retain flag: %d\r\n"
                    "Publish QoS: %d\r\n"
                    "Publish payload: %.*s \r\n",
                    pPublish->u.message.topicFilterLength,
                    pPublish->u.message.pTopicFilter,
                    pPublish->u.message.info.topicNameLength,
                    pPublish->u.message.info.pTopicName,
                    pPublish->u.message.info.retain,
                    pPublish->u.message.info.qos,
                    pPublish->u.message.info.payloadLength,
                    pPayload );
        
        APP_AWS_DBG(SYS_ERROR_DEBUG, "%.*s \r\n",pPublish->u.message.info.payloadLength, pPayload);
    
        cJSON *messageJson = cJSON_Parse(pPayload);
        if (messageJson == NULL) {
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL) {
                APP_AWS_DBG(SYS_ERROR_ERROR, "Message JSON parse Error. Error before: %s \r\n", error_ptr);
            }
            cJSON_Delete(messageJson);
            return;
        }

        //Get the desired state
        cJSON *state = cJSON_GetObjectItem(messageJson, "state");
        if (!state) {
            cJSON_Delete(messageJson);
            return;
        }
        
        //Get the toggle state
        cJSON *toggle = cJSON_GetObjectItem(state, "toggle");
        if (!toggle) {
            cJSON_Delete(messageJson);
            return;
        }

        bool desiredState = (bool)toggle->valueint;
        if (desiredState) {
            APP_AWS_PRNT("LED ON \r\n");
            APP_manageLed(LED_YELLOW, LED_S_BLINK_STARTING_ON, BLINK_MODE_SINGLE);
        } else {
            APP_AWS_PRNT("LED OFF \r\n");
            APP_manageLed(LED_YELLOW, LED_S_BLINK_STARTING_OFF, BLINK_MODE_SINGLE);
        }
        cJSON_Delete(messageJson);
        
        /* Publish LED state to shadow/update/ */
        appAwsData.shadowUpdate = true;
        appAwsData.publishToCloud = true;
    }
}

// *****************************************************************************

/* Transmit all messages and wait for them to be received on topic filters */
static int publishMessage()
{
    int status = 0;
    IotMqttError_t publishStatus = IOT_MQTT_STATUS_PENDING;
    IotMqttPublishInfo_t publishInfo = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
    IotMqttCallbackInfo_t publishComplete = IOT_MQTT_CALLBACK_INFO_INITIALIZER;
    char pPublishPayload[ APP_AWS_MAX_MSG_LLENGTH ];   
    char pubTopic[APP_AWS_TOPIC_NAME_MAX_LEN];
    const char * pPublishTopics[ PUBLISH_TOPIC_COUNT ] =
    {
        pubTopic,
    };
    
    if(appAwsData.shadowUpdate)
        snprintf(pPublishTopics[0], APP_AWS_TOPIC_NAME_MAX_LEN, APP_AWS_SHADOW_UPDATE_TOPIC_TEMPLATE, g_Aws_ClientID);
    else
        snprintf(pPublishTopics[0], APP_AWS_TOPIC_NAME_MAX_LEN, "%s/sensors", g_Aws_ClientID);

    publishComplete.function = operationCompleteCallback;
    publishComplete.pCallbackContext = NULL;

    /* Set the common members of the publish info. */
    publishInfo.qos = IOT_MQTT_QOS_1;
    publishInfo.topicNameLength = strlen(pPublishTopics[0]);
    publishInfo.pPayload = pPublishPayload;
    publishInfo.retryMs = PUBLISH_RETRY_MS;
    publishInfo.retryLimit = PUBLISH_RETRY_LIMIT;
    publishInfo.pTopicName = pPublishTopics[0];
    
    APP_AWS_DBG(SYS_ERROR_INFO, "Publishing message\r\n");

    /* Generate the payload for the PUBLISH. */
    if(appAwsData.shadowUpdate){
        status = snprintf( pPublishPayload, APP_AWS_MAX_MSG_LLENGTH, APP_AWS_SHADOW_MSG_TEMPLATE, !LED_YELLOW_Get());
        appAwsData.shadowUpdate = false;
    }
    else
#if 1
        status = snprintf( pPublishPayload, APP_AWS_MAX_MSG_LLENGTH,
                APP_AWS_TELEMETRY_MSG_TEMPLATE, 
                APP_readTemp(),
                APP_readLight());
#else
        /*Graduation step to include an additional sensor data. 
        Comment out the above code block by changing the '#if 1' to '#if 0'*/
        status = snprintf( pPublishPayload, APP_AWS_MAX_MSG_LLENGTH,
                APP_AWS_TELEMETRY_MSG_GRAD_TEMPLATE, 
                APP_readTemp(),
                APP_readLight(),
                !SWITCH1_Get());
#endif
    

    /* Check for errors from snprintf. */
    if( status < 0 ){
        APP_AWS_DBG(SYS_ERROR_ERROR, "Failed to generate MQTT PUBLISH payload for PUBLISH \r\n");
        return status;
    }
    else{
        publishInfo.payloadLength = ( size_t ) status;
        status = 1;
    }

    /* PUBLISH a message. This is an asynchronous function that notifies of
     * completion through a callback. */
    publishStatus = IotMqtt_PublishAsync( appAwsData.mqttConnection,
                                          &publishInfo,
                                          ( 0x80000000 ),
                                          &publishComplete,
                                          NULL );
    if( publishStatus != IOT_MQTT_STATUS_PENDING ){
        APP_AWS_DBG(SYS_ERROR_ERROR, "MQTT PUBLISH returned error %s \r\n", IotMqtt_strerror( publishStatus ) );
        status = 0;
    }
    appAwsData.pendingMessages++;

    return status;
}

// *****************************************************************************

void APP_AWS_Initialize( void )
{
    appAwsData.awsCloudTaskState = APP_AWS_CLOUD_SDK_INIT;
    appAwsData.mqttConnection = IOT_MQTT_CONNECTION_INITIALIZER;
    memset(g_Aws_ClientID, 0, CLIENT_IDENTIFIER_MAX_LENGTH);
    memset(g_Cloud_Endpoint, 0, 100);
    MQTT_DISCONNECTED;
    appAwsData.shadowUpdate = true;
    appAwsData.pubTimerHandle = SYS_TIME_HANDLE_INVALID;
    appAwsData.publishToCloud = false;
    appAwsData.pendingMessages = 0;
}

// *****************************************************************************

void APP_AWS_Tasks ( void )
{
    switch ( appAwsData.awsCloudTaskState )
    {
        /* AWS cloud task initial state. */
        case APP_AWS_CLOUD_SDK_INIT:
        {   		
			IotMqttError_t mqttInitStatus = IOT_MQTT_SUCCESS;
			bool sdkInitialized = false;

            /* Call the SDK initialization function. */
            sdkInitialized = IotSdk_Init();

            if( sdkInitialized == false ){
                APP_AWS_DBG(SYS_ERROR_ERROR, "Error occurred while SDK init \r\n" );
                appAwsData.awsCloudTaskState = APP_AWS_CLOUD_ERROR;
            }

			mqttInitStatus = IotMqtt_Init();
           
		    if( mqttInitStatus != IOT_MQTT_SUCCESS ){
		        /* Failed to initialize MQTT library. */
		        APP_AWS_DBG(SYS_ERROR_ERROR, "Error occurred while MQTT init \r\n" );
                appAwsData.awsCloudTaskState = APP_AWS_CLOUD_ERROR;
		    }
                       
            appAwsData.awsCloudTaskState = APP_AWS_CLOUD_PENDING;
            break;
        }
        
        /* AWS cloud task pending for WLAN task trigger */
        case APP_AWS_CLOUD_PENDING:
        {
            if(WIFI_IS_CONNECTED && IP_ADDR_IS_OBTAINED && NTP_IS_DONE){
                appAwsData.awsCloudTaskState = APP_AWS_CLOUD_MQTT_CONNECT;
            }
            break;
        }  

        /* MQTT connect */
        case APP_AWS_CLOUD_MQTT_CONNECT:
        {
            if(WIFI_IS_CONNECTED && IP_ADDR_IS_OBTAINED && NTP_IS_DONE){
                IotMqttError_t connectStatus = IOT_MQTT_STATUS_PENDING;
                int status = 0;
                struct IotNetworkServerInfo serverInfo = {0};
                IotMqttNetworkInfo_t networkInfo = IOT_MQTT_NETWORK_INFO_INITIALIZER;
                IotMqttConnectInfo_t connectInfo = IOT_MQTT_CONNECT_INFO_INITIALIZER;
                char pClientIdentifierBuffer[ CLIENT_IDENTIFIER_MAX_LENGTH ] = { 0 };

                if (g_Cloud_Endpoint[0] != NULL)
                    serverInfo.pHostName = g_Cloud_Endpoint;
                else{
                    APP_AWS_DBG(SYS_ERROR_ERROR, "endpoint null\r\n");
                    appAwsData.awsCloudTaskState = APP_AWS_CLOUD_ERROR;
                    break;
                }
                serverInfo.port = IOT_DEMO_PORT;

                /* Set the members of the network info not set by the initializer */
                networkInfo.createNetworkConnection = true;
                networkInfo.u.setup.pNetworkServerInfo = &serverInfo;
                networkInfo.u.setup.pNetworkCredentialInfo = NULL;
                networkInfo.pNetworkInterface = IOT_NETWORK_INTERFACE_WOLFSSL;

                /* Set MQTT disconnect callback*/
                networkInfo.disconnectCallback.function = mqttDisconnectCallback;
                networkInfo.disconnectCallback.pCallbackContext = NULL;

                /* Set the members of the connection info not set by the initializer. */
                connectInfo.awsIotMqttMode = true;
                connectInfo.cleanSession = true;
                connectInfo.keepAliveSeconds = KEEP_ALIVE_SECONDS;

                /* AWS mqtt doesn't use username or password */
                connectInfo.pUserName = NULL;
                connectInfo.userNameLength = 0;
                connectInfo.pPassword = NULL;
                connectInfo.passwordLength = 0;

                if (g_Aws_ClientID[0] != NULL){
                    status = sprintf(pClientIdentifierBuffer, "%s", g_Aws_ClientID);
                    if( status < 0 )
                        APP_AWS_DBG(SYS_ERROR_ERROR, "Failed to generate unique client identifier for demo \r\n" );
                    else{
                        /* Set the client identifier buffer and length. */
                        connectInfo.pClientIdentifier = pClientIdentifierBuffer;
                        connectInfo.clientIdentifierLength = ( uint16_t ) status;
                    }
                }
                else{
                    APP_AWS_DBG(SYS_ERROR_ERROR, "client id null\r\n");
                    appAwsData.awsCloudTaskState = APP_AWS_CLOUD_ERROR;
                    break;
                }

                /* Establish the MQTT connection. */
                APP_AWS_PRNT("MQTT connect .. \r\n");
                APP_AWS_DBG(SYS_ERROR_INFO, "MQTT client identifier is %.*s (length %u) \r\n",
                            connectInfo.clientIdentifierLength,
                            connectInfo.pClientIdentifier,
                            connectInfo.clientIdentifierLength );
                
                APP_manageLed(LED_GREEN, LED_F_BLINK, BLINK_MODE_PERIODIC);
                connectStatus = IotMqtt_Connect( &networkInfo,
                                                 &connectInfo,
                                                 MQTT_TIMEOUT_MS,
                                                 &appAwsData.mqttConnection );

                /* Back-off for 1 second before retry MQTT connection */
                if( connectStatus != IOT_MQTT_SUCCESS ){
                    APP_AWS_DBG(SYS_ERROR_ERROR, "MQTT CONNECT returned error %s \r\n",
                                 IotMqtt_strerror( connectStatus ) );
                    APP_manageLed(LED_GREEN, LED_OFF, BLINK_MODE_INVALID);
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                }
                else{
                    APP_AWS_PRNT("MQTT connected \r\n");
                    APP_manageLed(LED_GREEN, LED_ON, BLINK_MODE_INVALID);
                    APP_OLEDNotify(APP_OLED_PARAM_CLOUD, true);
                    appAwsData.awsCloudTaskState = APP_AWS_CLOUD_MQTT_SUBSCRIBE_TO_TOPIC;
                    MQTT_CONNECTED;
                }
            }
            else{
                /* Either Wi-Fi disconnected or IP address not assigned */
                appAwsData.awsCloudTaskState = APP_AWS_CLOUD_PENDING;
            }
			
            break;
        }

        /* Subscribe */
        case APP_AWS_CLOUD_MQTT_SUBSCRIBE_TO_TOPIC:
        {
			uint8_t i = 0;
		    IotMqttError_t subscriptionStatus = IOT_MQTT_STATUS_PENDING;
		    IotMqttSubscription_t pSubscriptions[ SUBSCRIBE_TOPIC_COUNT ] = { IOT_MQTT_SUBSCRIPTION_INITIALIZER };
            char subTopic[APP_AWS_TOPIC_NAME_MAX_LEN];
            char * pSubscribeTopics[ SUBSCRIBE_TOPIC_COUNT ] =
            {
                subTopic,
            };
            snprintf(subTopic, APP_AWS_TOPIC_NAME_MAX_LEN, APP_AWS_SHADOW_DELTA_TOPIC_TEMPLATE, g_Aws_ClientID);
            
		    /* Set the members of the subscription list */
		    for( i = 0; i < SUBSCRIBE_TOPIC_COUNT; i++ ){
		        pSubscriptions[i].qos = IOT_MQTT_QOS_1;
		        pSubscriptions[i].pTopicFilter = pSubscribeTopics[i];
		        pSubscriptions[i].topicFilterLength = strlen(pSubscribeTopics[i]);
		        pSubscriptions[i].callback.pCallbackContext = NULL;
		        pSubscriptions[i].callback.function = MqttCallback;
		    }
            
            if (MQTT_IS_CONNECTED){
                subscriptionStatus = IotMqtt_SubscribeSync( appAwsData.mqttConnection,
                                                        pSubscriptions,
                                                        SUBSCRIBE_TOPIC_COUNT,
                                                        0,
                                                        MQTT_TIMEOUT_MS );
                if ( subscriptionStatus == IOT_MQTT_SUCCESS){
                    APP_AWS_PRNT("MQTT subscriptions accepted \r\n" );
                    appAwsData.awsCloudTaskState = APP_AWS_CLOUD_MQTT_PUBLISH_TO_TOPIC;
                }
            }
            else
                /* MQTT disconnected */
                appAwsData.awsCloudTaskState = APP_AWS_CLOUD_PENDING;

            break;
        }
        
        /* Publish */
        case APP_AWS_CLOUD_MQTT_PUBLISH_TO_TOPIC:
        {
            if(appAwsData.pubTimerHandle == SYS_TIME_HANDLE_INVALID){
                appAwsData.pubTimerHandle = SYS_TIME_CallbackRegisterMS(pubTimerCallback, (uintptr_t) 0, PUBLISH_FREQUENCY_MS, SYS_TIME_PERIODIC);
                if (appAwsData.pubTimerHandle == SYS_TIME_HANDLE_INVALID) {
                    APP_AWS_DBG(SYS_ERROR_ERROR, "Failed creating a timer for periodic publish \r\n");
                    appAwsData.awsCloudTaskState = APP_AWS_CLOUD_ERROR;
                    break;
                }
            }
            
            if (MQTT_IS_CONNECTED){
                if(appAwsData.publishToCloud == true){
                    int status = 0;

                    /* Publish messages. */
                    status = publishMessage();
                    if (status == 0){
                        APP_AWS_DBG(SYS_ERROR_ERROR, "Publish message failed \r\n");
                        appAwsData.awsCloudTaskState = APP_AWS_CLOUD_ERROR;
                        break;
                    }
                    appAwsData.publishToCloud = false;
                }
            }
            else
                /* MQTT disconnected; try to re-connect only if ALL older messages are done */
                /* in context of a callback is received whether success or failure */
                if(appAwsData.pendingMessages == 0)
                    appAwsData.awsCloudTaskState = APP_AWS_CLOUD_PENDING;
            
            break;
        }
        
        /* Idle */
        case APP_AWS_CLOUD_IDLE:
        {
            break;
        }
        
        /* Error */
        case APP_AWS_CLOUD_ERROR:
        {
            SYS_CONSOLE_MESSAGE("APP_AWS_CLOUD_ERROR\r\n");
            appAwsData.awsCloudTaskState = APP_AWS_CLOUD_IDLE;
            break;
        }        

        default:
        {
            break;
        }
    }
}
#endif /* AWS_CLOUD_DEMO */

/*******************************************************************************
 End of File
 */
