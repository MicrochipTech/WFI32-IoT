/*
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file iot_network_wolfssl.c
 * @brief Implementation of the network interface functions in iot_network.h
 * for systems with WolfSSL.
 */

/* The config header is always included first. */
#include "iot_config.h"

/* Standard includes. */
#include <stdbool.h>
#include <string.h>



/* WolfSSL network include. */
#include "iot_network_wolfssl.h"

typedef int socklen_t ;

/* Platform threads include. */
#include "platform/iot_threads.h"

/* Error handling include. */
#include "iot_error.h"

#define TCP_CLIENT_CONNECTION_TIMEOUT_PERIOD_s 	10


/* Configure logs for the functions in this file. */
#ifdef IOT_LOG_LEVEL_NETWORK
    #define LIBRARY_LOG_LEVEL        IOT_LOG_LEVEL_NETWORK
#else
    #ifdef IOT_LOG_LEVEL_GLOBAL
        #define LIBRARY_LOG_LEVEL    IOT_LOG_LEVEL_GLOBAL
    #else
        #define LIBRARY_LOG_LEVEL    IOT_LOG_NONE
    #endif
#endif

#define LIBRARY_LOG_NAME    ( "NET" )
#include "iot_logging_setup.h"

/*
 * Provide default values for undefined memory allocation functions.
 */
#ifndef IotNetwork_Malloc
    #include <stdlib.h>

/**
 * @brief Memory allocation. This function should have the same signature
 * as [malloc](http://pubs.opengroup.org/onlinepubs/9699919799/functions/malloc.html).
 */
    #define IotNetwork_Malloc    malloc
#endif
#ifndef IotNetwork_Free
    #include <stdlib.h>

/**
 * @brief Free memory. This function should have the same signature as
 * [free](http://pubs.opengroup.org/onlinepubs/9699919799/functions/free.html).
 */
    #define IotNetwork_Free    free
#endif

uint32_t sockConnTimeStamp;
IP_MULTI_ADDRESS hostAddress;


/*-----------------------------------------------------------*/

/**
 * @brief Represents a network connection.
 */
typedef struct _networkConnection
{
    NET_PRES_SKT_HANDLE_T socket;                                  /**< @brief Socket associated with this connection. */
    bool receiveThreadCreated;                   /**< @brief Whether a receive thread exists for this connection. */
    TaskHandle_t  receiveThread;                     /**< @brief Thread that handles receiving on this connection. */
    IotNetworkReceiveCallback_t receiveCallback; /**< @brief Network receive callback, if any. */
    void * pReceiveContext;                      /**< @brief The context for the receive callback. */
    IotNetworkCloseCallback_t closeCallback;     /**< @brief Network close callback, if any. */
    void * pCloseContext;                        /**< @brief The context for the close callback. */
} _networkConnection_t;

/*-----------------------------------------------------------*/

/**
 * @brief An #IotNetworkInterface_t that uses the functions in this file.
 */
static const IotNetworkInterface_t _networkWolfSSL =
{
    .create             = IotNetworkWolfSSL_Create,
    .setReceiveCallback = IotNetworkWolfSSL_SetReceiveCallback,
    .setCloseCallback   = IotNetworkWolfSSL_SetCloseCallback,
    .send               = IotNetworkWolfSSL_Send,
    .receive            = IotNetworkWolfSSL_Receive,
    .close              = IotNetworkWolfSSL_Close,
    .destroy            = IotNetworkWolfSSL_Destroy
};

/*-----------------------------------------------------------*/

/**
 * @brief Network receive thread.
 *
 * This thread polls the network socket and reads data when data is available.
 * It then invokes the receive callback, if any.
 *
 * @param[in] pArgument The connection associated with this receive thread.
 */
static void * _networkReceiveThread( void * pArgument )
{
    int pollStatus = 0;


    /* Cast function parameter to correct type. */
    _networkConnection_t * pConnection = pArgument;

    
    /* Continuously poll the network socket for events. */
    while( true )
    {

        /* If an error event is detected, terminate this thread. */
        if((NET_PRES_SocketWasReset(pConnection->socket)) || (!NET_PRES_SocketIsConnected(pConnection->socket)))
        {
            IotLogError( "Detected error on socket %d, receive thread exiting.",
                         pConnection->socket );
            break;
        }

		pollStatus = NET_PRES_SocketReadIsReady(pConnection->socket);
		
		if(pollStatus>0)
        {
	        /* Invoke the callback function. */
	        pConnection->receiveCallback( pConnection,
	                                      pConnection->pReceiveContext );
        }
        else
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }

    /**
     * If a close callback has been defined, invoke it now; since we
     * don't know what caused the close, use "unknown" as the reason.
     */
    if( pConnection->closeCallback != NULL )
    {
        printf("enter ccb\r\n");
        pConnection->closeCallback( pConnection,
                                    IOT_NETWORK_UNKNOWN_CLOSED,
                                    pConnection->pCloseContext );
        printf("exit ccb\r\n");
    }
    else
    {
        IotLogDebug( "closeCallback == NULL" );
    }

    IotLogDebug( "Network receive thread for socket %d terminating.",
                 pConnection->socket );
    
    while( true )
    {
        // FreeRTOS task should not exit without deleting itself.
        // wait for IotNetworkWolfSSL_Close() function to delete this task.
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    return NULL;
}



/*-----------------------------------------------------------*/

/**
 * @brief Set up TLS on a TCP connection.
 *
 * @param[in] pConnection An established TCP connection.
 * @param[in] pServerName Remote host name, used for server name indication.
 * @param[in] pOpensslCredentials TLS setup parameters.
 *
 * @return #IOT_NETWORK_SUCCESS, #IOT_NETWORK_FAILURE, or #IOT_NETWORK_SYSTEM_ERROR.
 */
 #if 0
static IotNetworkError_t _tlsSetup( _networkConnection_t * pConnection,
                                    const char * pServerName,
                                    const IotNetworkCredentials_t pOpensslCredentials )
{
    IOT_FUNCTION_ENTRY( IotNetworkError_t, IOT_NETWORK_SUCCESS );
    

    /* Set up ALPN if requested. */
    if( pOpensslCredentials->pAlpnProtos != NULL )
    {
        IotLogDebug( "Setting ALPN protos." );

        if( ( SSL_set_alpn_protos( pConnection->pSslContext,
                                   ( const unsigned char * ) pOpensslCredentials->pAlpnProtos,
                                   ( unsigned int ) strlen( pOpensslCredentials->pAlpnProtos ) ) != 0 ) )
        {
            IotLogError( "Failed to set ALPN protos." );

            IOT_SET_AND_GOTO_CLEANUP( IOT_NETWORK_SYSTEM_ERROR );
        }
    }

    /* Set TLS MFLN if requested. */
    if( pOpensslCredentials->maxFragmentLength > 0 )
    {
        IotLogDebug( "Setting max send fragment length %lu.",
                     ( unsigned long ) pOpensslCredentials->maxFragmentLength );

        /* Set the maximum send fragment length. */
        if( SSL_set_max_send_fragment( pConnection->pSslContext,
                                       ( long ) pOpensslCredentials->maxFragmentLength ) != 1 )
        {
            IotLogError( "Failed to set max send fragment length %lu.",
                         ( unsigned long ) pOpensslCredentials->maxFragmentLength );

            IOT_SET_AND_GOTO_CLEANUP( IOT_NETWORK_SYSTEM_ERROR );
        }


    }

    /* Enable SNI if requested. */
    if( pOpensslCredentials->disableSni == false )
    {
        IotLogDebug( "Setting server name %s for SNI.", pServerName );

        if( SSL_set_tlsext_host_name( pConnection->pSslContext,
                                      pServerName ) != 1 )
        {
            IotLogError( "Failed to set server name %s for SNI.", pServerName );

            IOT_SET_AND_GOTO_CLEANUP( IOT_NETWORK_SYSTEM_ERROR );
        }
    }



    IOT_FUNCTION_CLEANUP_BEGIN();

    IOT_FUNCTION_CLEANUP_END();
}

#endif

/*-----------------------------------------------------------*/

/**
 * @brief Perform a DNS lookup of a host name and establish a TCP connection.
 *
 * @param[in] pServerInfo Server host name and port.
 *
 * @return A connected TCP socket number; `-1` if the DNS lookup failed.
 */


static int _dnsLookupAndConnect( IotNetworkServerInfo_t pServerInfo, IotNetworkCredentials_t pCredentialInfo)
{
    IOT_FUNCTION_ENTRY( int, 0 );
    int16_t tcpSocket = -1;
    const uint16_t netPort = ( pServerInfo->port );
    struct addrinfo * pListHead = NULL, * pAddressInfo = NULL;
    struct sockaddr * pServer = NULL;
    socklen_t serverLength = 0;

    /* Perform a DNS lookup of host name. */
    IotLogInfo( "Performing DNS lookup of %s, %d", pServerInfo->pHostName , strlen(pServerInfo->pHostName ));

	/* First check to see if host is an IPv4 address*/
    if (TCPIP_Helper_StringToIPAddress( pServerInfo->pHostName, &hostAddress.v4Add))
    {
    	//string is already in IPv4 format
        IotLogDebug("Using IPv4 Address: %d.%d.%d.%d for host '%s'\r\n", hostAddress.v4Add.v[0], hostAddress.v4Add.v[1], hostAddress.v4Add.v[2], hostAddress.v4Add.v[3], pServerInfo->pHostName);
    }
    else
    {
    	//perform DNS query

		TCPIP_DNS_RESULT result;
        result = TCPIP_DNS_Resolve(pServerInfo->pHostName, TCPIP_DNS_TYPE_A);

		 if (result >= 0)
         {
         	result = TCPIP_DNS_RES_PENDING;

			while(result == TCPIP_DNS_RES_PENDING)
			{
	            result = TCPIP_DNS_IsResolved(pServerInfo->pHostName, &hostAddress, IP_ADDRESS_TYPE_IPV4);

	            switch (result)
	            {
	                case TCPIP_DNS_RES_PENDING:
	                {
						// wait a second, not optimum, todo, optimize this 
						vTaskDelay(1000 / portTICK_PERIOD_MS);
	                    break;
	                }
	                
	                case TCPIP_DNS_RES_OK:
	                {
	                    IotLogDebug("Using IPv4 Address: %d.%d.%d.%d for host '%s'\r\n", hostAddress.v4Add.v[0], hostAddress.v4Add.v[1], hostAddress.v4Add.v[2], hostAddress.v4Add.v[3], pServerInfo->pHostName);
	                    break;
	                }
	                
	                case TCPIP_DNS_RES_SERVER_TMO:
	                case TCPIP_DNS_RES_NO_IP_ENTRY:
	                default:
	                {
	                    IotLogError( "DNS lookup failed.. " );
						IOT_SET_AND_GOTO_CLEANUP( -1 );
	                    break;
	                }
	            }
			}
         }
         else
         {
                IotLogError( "DNS lookup failed.. %d", result );
				IOT_SET_AND_GOTO_CLEANUP( -1 );
         }            

    }

    IotLogDebug( "Successfully received DNS address." );


	// If we're here it means that we have a proper address.
	NET_PRES_SKT_ERROR_T error;
	IotLogDebug("Starting TCP/IPv4 Connection to : %d.%d.%d.%d port '%d'\r\n", hostAddress.v4Add.v[0], hostAddress.v4Add.v[1], hostAddress.v4Add.v[2], hostAddress.v4Add.v[3], netPort);
	tcpSocket = NET_PRES_SocketOpen(0, NET_PRES_SKT_UNENCRYPTED_STREAM_CLIENT, IP_ADDRESS_TYPE_IPV4, netPort, (NET_PRES_ADDRESS *)&hostAddress, &error);
    NET_PRES_SocketWasReset(tcpSocket);
	if (tcpSocket == INVALID_SOCKET)
	{
		IotLogError("Error %d: Could not create socket - aborting\r\n", error);
		IOT_SET_AND_GOTO_CLEANUP( -1 );
	}
	else
	{
		IotLogDebug("Starting connection\r\n");
		sockConnTimeStamp = SYS_TMR_TickCountGet();
	}

    while( !NET_PRES_SocketIsConnected(tcpSocket))
        {
        	// while Socket not connected 
           if (SYS_TMR_TickCountGet() - sockConnTimeStamp >= SYS_TMR_TickCounterFrequencyGet() * TCP_CLIENT_CONNECTION_TIMEOUT_PERIOD_s)
            {
                IotLogError("Socket connect timeout!\r\n");
                TCPIP_TCP_Close(tcpSocket);
				IOT_SET_AND_GOTO_CLEANUP( -1 );
            }
		   vTaskDelay(100 / portTICK_PERIOD_MS);
            
        }


	// socket connected, setup TLS
	//TODO: add support for ALPN and SNI
    /*if( pCredentialInfo != NULL )
    {
        IotLogInfo( "Setting up TLS." );



       _tlsSetup( pNewNetworkConnection,
                            pServerInfo->pHostName,
                            pCredentialInfo );
    }*/
	
		IotLogDebug("Connection Opened: Starting SSL Negotiation\r\n");
        

		if (!NET_PRES_SocketEncryptSocket(tcpSocket)) 
 		{
			IotLogError("SSL Create Connection Failed - Aborting\r\n");
            TCPIP_TCP_Close(tcpSocket);
			IOT_SET_AND_GOTO_CLEANUP( -1 );                
		} 

        
    while( NET_PRES_SocketIsNegotiatingEncryption(tcpSocket))
		vTaskDelay(10 / portTICK_PERIOD_MS);

	if (!NET_PRES_SocketIsSecure(tcpSocket)) 
	{
		IotLogError("SSL Connection Negotiation Failed - Aborting\r\n");
		TCPIP_TCP_Close(tcpSocket);
		IOT_SET_AND_GOTO_CLEANUP( -1 );
	}

	IotLogDebug("SSL Connection Opened\r\n");
 

    IOT_FUNCTION_CLEANUP_BEGIN();



    /* Return the socket descriptor on success. */
    if( status == 0 )
    {
        status = tcpSocket;
    }

    IOT_FUNCTION_CLEANUP_END();
}

/*-----------------------------------------------------------*/



const IotNetworkInterface_t * IotNetworkWolfSSL_GetInterface( void )
{
    return &_networkWolfSSL;
}

/*-----------------------------------------------------------*/


IotNetworkError_t IotNetworkWolfSSL_Create( IotNetworkServerInfo_t pServerInfo,
                                            IotNetworkCredentials_t pCredentialInfo,
                                            IotNetworkConnection_t * pConnection )
{
    IOT_FUNCTION_ENTRY( IotNetworkError_t, IOT_NETWORK_SUCCESS );
    int tcpSocket = -1;
    bool sslMutexCreated = false;
    _networkConnection_t * pNewNetworkConnection = NULL;

    /* Allocate memory for a new connection. */
    pNewNetworkConnection = IotNetwork_Malloc( sizeof( _networkConnection_t ) );

    if( pNewNetworkConnection == NULL )
    {
        IotLogError( "Failed to allocate memory for new network connection." );

        IOT_SET_AND_GOTO_CLEANUP( IOT_NETWORK_NO_MEMORY );
    }

    /* Clear connection data. */
    ( void ) memset( pNewNetworkConnection, 0x00, sizeof( _networkConnection_t ) );

    /* Perform a DNS lookup of pHostName. This also establishes a TCP  & TLS socket. */
    tcpSocket = _dnsLookupAndConnect( pServerInfo , pCredentialInfo);

    if( tcpSocket == -1 )
    {
        IOT_SET_AND_GOTO_CLEANUP( IOT_NETWORK_FAILURE );
    }
    else
    {
        IotLogInfo( "TCP connection successful." );
    }

    /* Set the socket in the network connection. */
    pNewNetworkConnection->socket = tcpSocket;


    /* Clean up on error. */
    IOT_FUNCTION_CLEANUP_BEGIN();

    if( status != IOT_NETWORK_SUCCESS )
    {
        if( tcpSocket != -1 )
        {
            ( void ) NET_PRES_SocketClose( tcpSocket );
            tcpSocket = -1;
        }

        if( pNewNetworkConnection != NULL )
        {
            IotNetwork_Free( pNewNetworkConnection );
        }
    }
    else
    {
        /* Set the output parameter. */
        *pConnection = pNewNetworkConnection;
    }

    IOT_FUNCTION_CLEANUP_END();
}

/*-----------------------------------------------------------*/

IotNetworkError_t IotNetworkWolfSSL_SetReceiveCallback( IotNetworkConnection_t pConnection,
                                                        IotNetworkReceiveCallback_t receiveCallback,
                                                        void * pContext )
{
    IotNetworkError_t status = IOT_NETWORK_BAD_PARAMETER;
	BaseType_t  res;

    /* The receive callback must be non-NULL) */
    if( receiveCallback != NULL )
    {
        /* Set the callback and parameter. */
        pConnection->receiveCallback = receiveCallback;
        pConnection->pReceiveContext = pContext;
        
        IotLogInfo( "xTaskCreate recv thread" );

		res = xTaskCreate( _networkReceiveThread, "Receive Thread" , 1024, pConnection, 1,&(pConnection->receiveThread));
        
        if( res != pdPASS  )
        {
            IotLogError( "Failed to create socket %d network receive thread. errno=%d.",
                         pConnection->socket,
                         res );
            status = IOT_NETWORK_SYSTEM_ERROR;
        }
        else
        {
            pConnection->receiveThreadCreated = true;
            status = IOT_NETWORK_SUCCESS;
        }

    }

    return status;
}

/*-----------------------------------------------------------*/

IotNetworkError_t IotNetworkWolfSSL_SetCloseCallback( IotNetworkConnection_t pConnection,
                                                      IotNetworkCloseCallback_t closeCallback,
                                                      void * pContext )
{
    IotNetworkError_t status = IOT_NETWORK_BAD_PARAMETER;

    if( closeCallback != NULL )
    {
        /* Set the callback and parameter. */
        pConnection->closeCallback = closeCallback;
        pConnection->pCloseContext = pContext;

        status = IOT_NETWORK_SUCCESS;
    }

    return status;
}

/*-----------------------------------------------------------*/

size_t IotNetworkWolfSSL_Send( IotNetworkConnection_t pConnection,
                               const uint8_t * pMessage,
                               size_t messageLength )
{
    int bytesSent = 0;


    IotLogDebug( "Sending %lu bytes over socket %d.",
                 ( unsigned long ) messageLength,
                 pConnection->socket );


	/* Check that it's possible to send data right now. */
	if (NET_PRES_SocketWriteIsReady(pConnection->socket, messageLength, messageLength) ) 
    {
            bytesSent = NET_PRES_SocketWrite(pConnection->socket, pMessage, messageLength);
    }
    else
    {
        IotLogError( "Not possible to send on socket %d.", pConnection->socket );
    }


    /* Log the number of bytes sent. */
    if( bytesSent <= 0 )
    {
        IotLogError( "Send failure." );

        bytesSent = 0;
    }
    else if( ( size_t ) bytesSent != messageLength )
    {
        IotLogWarn( "Failed to send %lu bytes, %d bytes sent instead.",
                    ( unsigned long ) messageLength,
                    bytesSent );
    }
    else
    {
        IotLogDebug( "Successfully sent %lu bytes.",
                     ( unsigned long ) messageLength );
    }

    return ( size_t ) bytesSent;
}

/*-----------------------------------------------------------*/

size_t IotNetworkWolfSSL_Receive( IotNetworkConnection_t pConnection,
                                  uint8_t * pBuffer,
                                  size_t bytesRequested )
{
    int recv_count = 0;
    int bytesRead = 0;
    int bytesRemaining = (int) bytesRequested;

    IotLogDebug( "Blocking to wait for %lu bytes on socket %d.",
                 ( unsigned long ) bytesRequested,
                 pConnection->socket );



    /* Loop until all bytes are received. */
    while( bytesRemaining > 0 )
    {
        if ((!NET_PRES_SocketWasReset(pConnection->socket)) && (NET_PRES_SocketIsConnected(pConnection->socket)))
        {
            recv_count = NET_PRES_SocketRead(pConnection->socket, pBuffer+bytesRead, bytesRemaining);

            if (recv_count < bytesRemaining)
            {
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }

            bytesRead += recv_count;
            bytesRemaining -= recv_count;
        }
        else
        {
            break;
        }
    }
	

    /* Check how many bytes were read. */
    if( bytesRead < bytesRequested )
    {
        IotLogWarn( "Receive requested %lu bytes, but only %lu bytes received.",
                    ( unsigned long ) bytesRequested,
                    ( unsigned long ) bytesRead );
    }
    else
    {
        IotLogDebug( "Successfully received %lu bytes.",
                     ( unsigned long ) bytesRequested );
    }

    return bytesRead;
}

/*-----------------------------------------------------------*/

IotNetworkError_t IotNetworkWolfSSL_Close( IotNetworkConnection_t pConnection )
{
    if (pConnection == NULL)
    {
        return IOT_NETWORK_BAD_PARAMETER;
    }

    /* Check if a receive thread needs to be cleaned up. */
    if( pConnection->receiveThreadCreated == true )
    {
        vTaskDelete(pConnection->receiveThread);
        pConnection->receiveThreadCreated = false;
    }
    
    if( pConnection->socket != -1 )
    {
        IotLogInfo( "Connection (socket %d) shutting down.",
                    pConnection->socket );
        NET_PRES_SocketClose(pConnection->socket);
        pConnection->socket = -1;
    }

	sockConnTimeStamp = 0;

    return IOT_NETWORK_SUCCESS;
}

/*-----------------------------------------------------------*/

IotNetworkError_t IotNetworkWolfSSL_Destroy( IotNetworkConnection_t pConnection )
{
    if (pConnection == NULL)
    {
        return IOT_NETWORK_BAD_PARAMETER;
    }

    /* Close the socket file descriptor. */
    IotNetworkWolfSSL_Close(pConnection);

    /* Free the connection. */
    IotNetwork_Free( pConnection );

    return IOT_NETWORK_SUCCESS;
}

/*-----------------------------------------------------------*/

int IotNetworkWolfSSL_GetSocket( IotNetworkConnection_t pConnection )
{
    return pConnection->socket;
}

/*-----------------------------------------------------------*/
