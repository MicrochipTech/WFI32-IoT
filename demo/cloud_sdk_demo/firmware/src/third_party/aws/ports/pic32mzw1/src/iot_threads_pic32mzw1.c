/*
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * @file iot_threads_template.c
 * @brief Template implementation of the functions in iot_threads.h
 */

/* The config header is always included first. */
#include "iot_config.h"

/* Platform threads include. */
#include "platform/iot_threads.h"

/* Configure logs for the functions in this file. */
#ifdef IOT_LOG_LEVEL_PLATFORM
    #define LIBRARY_LOG_LEVEL        IOT_LOG_LEVEL_PLATFORM
#else
    #ifdef IOT_LOG_LEVEL_GLOBAL
        #define LIBRARY_LOG_LEVEL    IOT_LOG_LEVEL_GLOBAL
    #else
        #define LIBRARY_LOG_LEVEL    IOT_LOG_NONE
    #endif
#endif

#define LIBRARY_LOG_NAME    ( "THREAD" )
#include "iot_logging_setup.h"

/*-----------------------------------------------------------*/

bool Iot_CreateDetachedThread( IotThreadRoutine_t threadRoutine,
                               void * pArgument,
                               int32_t priority,
                               size_t stackSize )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_threads_function_createdetachedthread.html
     */
	bool xReturned = pdFALSE;
	
	TaskHandle_t xHandle = NULL;

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
                    threadRoutine,       /* Function that implements the task. */
                    "NAME",          /* Text name for the task. */
                    stackSize,      /* Stack size in words, not bytes. */
                    pArgument,    /* Parameter passed into the task. */
                    priority,/* Priority at which the task is created. */
                    &xHandle );      /* Used to pass out the created task's handle. */
	
    return xReturned;
}

/*-----------------------------------------------------------*/

bool IotMutex_Create( IotMutex_t * pNewMutex, bool recursive )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_threads_function_mutexcreate.html
     */

	bool xReturned = pdFALSE;
    _IotSystemMutex_t * internalMutex = ( _IotSystemMutex_t * ) pNewMutex;
    
    if (internalMutex == NULL)
    {
        IotLogError( "IotMutex_Create with NULL arg" );
        while(1);
    }
	
	/* Create a mutex type semaphore. */
	if(recursive)
	{
        internalMutex->isRecursive = pdTRUE;
		internalMutex->xMutex = xSemaphoreCreateRecursiveMutex();
	}
	else
    {
        internalMutex->isRecursive = pdFALSE;
        internalMutex->xMutex = xSemaphoreCreateMutex();
    }

   if( internalMutex->xMutex != NULL )
   {
       /* The semaphore was created successfully and
       can be used. */
       xReturned = pdTRUE;
   }
    return xReturned;
}

/*-----------------------------------------------------------*/

void IotMutex_Destroy( IotMutex_t * pMutex )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_threads_function_mutexdestroy.html
     */
     _IotSystemMutex_t * internalMutex = ( _IotSystemMutex_t * ) pMutex;
     
    if (internalMutex == NULL || internalMutex->xMutex == NULL)
    {
        IotLogError( "IotMutex_Destroy with NULL arg" );
        while(1);
    }

     vSemaphoreDelete( ( SemaphoreHandle_t ) internalMutex->xMutex );
}

/*-----------------------------------------------------------*/

bool prIotMutexTimedLock( IotMutex_t * pMutex,
                          TickType_t timeout )
{
    _IotSystemMutex_t * internalMutex = ( _IotSystemMutex_t * ) pMutex;
    BaseType_t lockResult;

    if (internalMutex == NULL || internalMutex->xMutex == NULL)
    {
        IotLogError( "prIotMutexTimedLock with NULL arg" );
        while(1);
    }

    IotLogDebug( "Locking mutex %p.", internalMutex );

    /* Call the correct FreeRTOS mutex take function based on mutex type. */
    if( internalMutex->isRecursive == pdTRUE )
    {
        lockResult = xSemaphoreTakeRecursive( ( SemaphoreHandle_t ) internalMutex->xMutex, timeout );
    }
    else
    {
        lockResult = xSemaphoreTake( ( SemaphoreHandle_t ) internalMutex->xMutex, timeout );
    }
    
    if ( lockResult == pdFALSE )
    {
        IotLogError("mutex lock operation failed");
        while(1);
    }

    return lockResult;
}

void IotMutex_Lock( IotMutex_t * pMutex )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_threads_function_mutexlock.html
     */
    if (pMutex == NULL)
    {
        IotLogError( "IotMutex_Lock with NULL arg" );
        while(1);
    }
    
    prIotMutexTimedLock( pMutex, portMAX_DELAY );
}

/*-----------------------------------------------------------*/

bool IotMutex_TryLock( IotMutex_t * pMutex )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_threads_function_mutextrylock.html
     */
    if (pMutex == NULL)
    {
        IotLogError( "IotMutex_TryLock with NULL arg" );
        while(1);
    }
    
    return prIotMutexTimedLock( pMutex, 0 );
}

/*-----------------------------------------------------------*/

void IotMutex_Unlock( IotMutex_t * pMutex )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_threads_function_mutexunlock.html
     */
	 _IotSystemMutex_t * internalMutex = ( _IotSystemMutex_t * ) pMutex;

    if (internalMutex == NULL || internalMutex->xMutex == NULL)
    {
        IotLogError( "IotMutex_Unlock with NULL arg" );
        while(1);
    }

    IotLogDebug( "Unlocking mutex %p.", internalMutex );

    /* Call the correct FreeRTOS mutex unlock function based on mutex type. */
    if( internalMutex->isRecursive == pdTRUE )
    {
        ( void ) xSemaphoreGiveRecursive( ( SemaphoreHandle_t ) internalMutex->xMutex );
    }
    else
    {
        ( void ) xSemaphoreGive( ( SemaphoreHandle_t ) internalMutex->xMutex );
    }
}

/*-----------------------------------------------------------*/

bool IotSemaphore_Create( IotSemaphore_t * pNewSemaphore,
                          uint32_t initialValue,
                          uint32_t maxValue )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_threads_function_semaphorecreate.html
     */
    bool xReturned = pdFALSE;

	/* Create a mutex type semaphore. */
   *pNewSemaphore = xSemaphoreCreateCounting( maxValue, initialValue ); 

   if( *pNewSemaphore != NULL )
   {
       /* The semaphore was created successfully and
       can be used. */
       xReturned = pdTRUE;
   }
    return xReturned;
}

/*-----------------------------------------------------------*/

void IotSemaphore_Destroy( IotSemaphore_t * pSemaphore )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_threads_function_semaphoredestroy.html
     */
    if (pSemaphore != NULL && *pSemaphore != NULL)
    {
        vSemaphoreDelete(*pSemaphore);
    }
    else
    {
        IotLogError( "IotSemaphore_Destroy with NULL arg" );
        while(1);
    }

}

/*-----------------------------------------------------------*/

uint32_t IotSemaphore_GetCount( IotSemaphore_t * pSemaphore )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_threads_function_semaphoregetcount.html
     */
    if (pSemaphore != NULL && *pSemaphore != NULL)
    {
        return uxSemaphoreGetCount( *pSemaphore );
    }
    else
    {
        IotLogError( "IotSemaphore_GetCount with NULL arg" );
        while(1);
    }
}

/*-----------------------------------------------------------*/

void IotSemaphore_Wait( IotSemaphore_t * pSemaphore )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_threads_function_semaphorewait.html
     */

    if (pSemaphore != NULL && *pSemaphore != NULL)
    {
        if (xSemaphoreTake( *pSemaphore, portMAX_DELAY ) != pdTRUE)
        {
            IotLogError( "IotSemaphore_Wait failed" );
            while(1);
        }
    }
    else
    {
        IotLogError( "IotSemaphore_Wait with NULL arg" );
        while(1);
    }
}

/*-----------------------------------------------------------*/

bool IotSemaphore_TryWait( IotSemaphore_t * pSemaphore )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_threads_function_semaphoretrywait.html
     */
    if (pSemaphore != NULL && *pSemaphore != NULL)
    {
        return IotSemaphore_TimedWait( pSemaphore, 0 );
    }
    else
    {
        IotLogError( "IotSemaphore_TryWait with NULL arg" );
        while(1);
    }
}

/*-----------------------------------------------------------*/

bool IotSemaphore_TimedWait( IotSemaphore_t * pSemaphore,
                             uint32_t timeoutMs )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_threads_function_semaphoretimedwait.html
     */
    	bool xReturned = pdFALSE;
    
    if (pSemaphore != NULL && *pSemaphore != NULL)
    {
        if( xSemaphoreTake( *pSemaphore, pdMS_TO_TICKS( timeoutMs ) ) == pdTRUE )
        {
            xReturned = pdTRUE;
        }
        else
        {
            /* We could not obtain the semaphore and can therefore not access
            the shared resource safely. */
            if( timeoutMs > 0 )
            {
                IotLogError( "Timeout waiting on semaphore" );
            }
        }
    }
    else
    {
        IotLogError( "IotSemaphore_TimedWait with NULL arg" );
        while(1);
    }
		
    return xReturned;
}

/*-----------------------------------------------------------*/

void IotSemaphore_Post( IotSemaphore_t * pSemaphore )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_threads_function_semaphorepost.html
     */

    if (pSemaphore != NULL && *pSemaphore != NULL)
    {
        xSemaphoreGive( *pSemaphore );
    }
    else
    {
        IotLogError( "IotSemaphore_Post with NULL arg" );
        while(1);
    }
}

/*-----------------------------------------------------------*/
