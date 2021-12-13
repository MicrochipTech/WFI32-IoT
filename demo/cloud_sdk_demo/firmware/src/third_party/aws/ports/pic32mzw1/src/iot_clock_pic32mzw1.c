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
 * @file iot_clock_template.c
 * @brief Template implementation of the functions in iot_clock.h
 */

/* The config header is always included first. */
#include "iot_config.h"

/* Platform clock include. */
#include "iot_clock.h"

#include "timers.h"

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

#define LIBRARY_LOG_NAME    ( "CLOCK" )
#include "iot_logging_setup.h"

/*-----------------------------------------------------------*/

/*
 * Time conversion constants.
 */
#define _MILLISECONDS_PER_SECOND    ( 1000 )                                          /**< @brief Milliseconds per second. */
#define _MILLISECONDS_PER_TICK      ( _MILLISECONDS_PER_SECOND / configTICK_RATE_HZ ) /**< Milliseconds per FreeRTOS tick. */

/*-----------------------------------------------------------*/

/*  Private Callback function for timer expiry, delegate work to a Task to free
 *  up the timer task for managing other timers */
static void prvTimerCallback( TimerHandle_t xTimerHandle )
{
    _IotSystemTimer_t * pxTimer = ( _IotSystemTimer_t * ) pvTimerGetTimerID( xTimerHandle );

    /* The value of the timer ID, set in timer_create, should not be NULL. */
    configASSERT( pxTimer != NULL );

    /* Restart the timer if it is periodic. */
    if( pxTimer->xTimerPeriod > 0 )
    {
        xTimerChangePeriod( xTimerHandle, pxTimer->xTimerPeriod, 0 );
    }

    /* Call timer Callback from this task */
    pxTimer->threadRoutine( ( void * ) pxTimer->pArgument );
}

bool IotClock_GetTimestring( char * pBuffer,
                             size_t bufferSize,
                             size_t * pTimestringLength )
{
    uint64_t milliSeconds = IotClock_GetTimeMs();
    int timestringLength = 0;

    configASSERT( pBuffer != NULL );
    configASSERT( pTimestringLength != NULL );

    /* Convert the localTime struct to a string. */
    timestringLength = snprintf( pBuffer, bufferSize, "%llu", milliSeconds );

    /* Check for error from no string */
    if( timestringLength == 0 )
    {
        return false;
    }

    /* Set the output parameter. */
    *pTimestringLength = timestringLength;

    return true;
}

/*-----------------------------------------------------------*/

uint64_t IotClock_GetTimeMs( void )
{
    TimeOut_t xCurrentTime = { 0 };

    /* This must be unsigned because the behavior of signed integer overflow is undefined. */
    uint64_t ullTickCount = 0ULL;

    /* Get the current tick count and overflow count. vTaskSetTimeOutState()
     * is used to get these values because they are both static in tasks.c. */
    vTaskSetTimeOutState( &xCurrentTime );

    /* Adjust the tick count for the number of times a TickType_t has overflowed. */
    ullTickCount = ( uint64_t ) ( xCurrentTime.xOverflowCount ) << ( sizeof( TickType_t ) * 8 );

    /* Add the current tick count. */
    ullTickCount += xCurrentTime.xTimeOnEntering;

    /* Return the ticks converted to Milliseconds */
    return ullTickCount * _MILLISECONDS_PER_TICK;
}

/*-----------------------------------------------------------*/

void IotClock_SleepMs( uint32_t sleepTimeMs )
{
    vTaskDelay( pdMS_TO_TICKS( sleepTimeMs ) );
}

/*-----------------------------------------------------------*/

bool IotClock_TimerCreate( IotTimer_t * pNewTimer,
                           IotThreadRoutine_t expirationRoutine,
                           void * pArgument )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_clock_function_timercreate.html
     */
	bool res = pdFALSE;
    _IotSystemTimer_t * pxTimer = ( _IotSystemTimer_t * ) pNewTimer;

	if(pNewTimer!= NULL && expirationRoutine != NULL)
	{
        /* Set the timer expiration routine, argument and period */
        pxTimer->threadRoutine = expirationRoutine;
        pxTimer->pArgument = pArgument;
        pxTimer->xTimerPeriod = 0;
    
		pxTimer->timer = xTimerCreate( "Timer",
                     /* Dummy period, modify on IotClock_TimerArm call */
                     portMAX_DELAY,
                     /* The timers will not auto-reload themselves
                     when they expire. */
                     pdFALSE,
                     ( void * ) pxTimer,
                     prvTimerCallback);

        if (pxTimer->timer != NULL)
        {
            res = pdTRUE;
        }
	}
    return res;
}

/*-----------------------------------------------------------*/

void IotClock_TimerDestroy( IotTimer_t * pTimer )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_clock_function_timerdestroy.html
     */
    _IotSystemTimer_t * pTimerInfo = ( _IotSystemTimer_t * ) pTimer;

	xTimerDelete( pTimerInfo->timer, portMAX_DELAY );
}

/*-----------------------------------------------------------*/

bool IotClock_TimerArm( IotTimer_t * pTimer,
                        uint32_t relativeTimeoutMs,
                        uint32_t periodMs )
{
    /* Implement this function as specified here:
     * https://docs.aws.amazon.com/freertos/latest/lib-ref/c-sdk/platform/platform_clock_function_timerarm.html
     */

	bool res = pdFALSE;
    _IotSystemTimer_t * pTimerInfo = ( _IotSystemTimer_t * ) pTimer;
    
    TimerHandle_t xTimerHandle = pTimerInfo->timer;
    
    /* Set the timer period in ticks */
    pTimerInfo->xTimerPeriod = pdMS_TO_TICKS( periodMs );

	/* xTimer is not active, change its period to 500ms.  This will also
        cause the timer to start.  Block for a maximum of 100 ticks if the
        change period command cannot immediately be sent to the timer
        command queue. */
        if( xTimerChangePeriod( xTimerHandle, pdMS_TO_TICKS( relativeTimeoutMs ), portMAX_DELAY )
                                                            == pdPASS )
        {
            /* The command was successfully sent. */
			res = pdTRUE;
        }
        else
        {
            /* The command could not be sent, even after waiting for 100 ticks
            to pass.  Take appropriate action here. */
        }
		
    return res;
}

/*-----------------------------------------------------------*/
